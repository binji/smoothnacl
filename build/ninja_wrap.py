#!/usr/bin/env python
import collections
import cStringIO
import glob
import itertools
import optparse
import os
import re
import string
import sys
import unittest

import ninja_syntax


def _AsList(v):
  if v is None:
    return []
  if isinstance(v, list):
    return v
  if isinstance(v, tuple):
    return list(v)
  return [v]


def Prefix(s, vs):
  if isinstance(vs, list):
    return [s + v for v in vs]
  return s + vs


def GlobList(*args):
  assert all(isinstance(arg, str) for arg in args)
  return reduce(lambda l, x: l + glob.glob(x), args, [])


def Exclude(vs, *regexs):
  new_vs = []
  regexps = _AsList(regexs)
  for v in _AsList(vs):
    if not any(re.search(r, v) for r in regexs):
      new_vs.append(v)
  return new_vs


class Formatter(string.Formatter):
  # Borrowed from python stdlib's string.py
  def vformat(self, format_string, args, kwargs):
    result = []
    for literal_text, field_name, format_spec, conversion in \
            self.parse(format_string):
      # output the literal text
      if literal_text:
        result.append(literal_text)

      # if there's a field, output it
      if field_name is not None:
        # this is some markup, find the object and do
        #  the formatting

        # given the field_name, find the object it references
        #  and the argument it came from
        try:
          obj, arg_used = self.get_field(field_name, args, kwargs)
        except:
          msg = field_name
          if conversion:
            msg += '!' + conversion
          if format_spec:
            msg += ':' + format_spec
          result.append('{%s}' % msg)
        else:
          # do any conversion on the resulting object
          obj = self.convert_field(obj, conversion)

          # format the object and append to the result
          result.append(self.format_field(obj, format_spec))

    return ''.join(result)

  def format_field(self, obj, format_spec):
    if format_spec == '-ext':
      assert isinstance(obj, str)
      return os.path.splitext(obj)[0]
    return format(obj, format_spec)


class Object(object):
  def __init__(self, context, keys=None, **kwargs):
    assert isinstance(keys, (dict, type(None)))
    self.context = context
    self.keys = keys
    self.args = kwargs
    self.tags = set()

  def __getattr__(self, key):
    return self.args.get(key)

  def __setattr__(self, key, value):
    assert key in ('context', 'args', 'tags', 'keys')
    object.__setattr__(self, key, value)

  def __str__(self):
    return '<%s: keys: %r args: %r tags: %r>' % (
        self.__class__.__name__, self.keys, self.args, self.tags)

  def __repr__(self):
    return '<%s: keys: %r args: %r tags: %r>' % (
        self.__class__.__name__, self.keys, self.args, self.tags)

  def Set(self, key, values):
    if isinstance(values, list):
      assert all(isinstance(v, str) for v in values)
      return self.Append(key, values)
    elif isinstance(values, str):
      self.args[key] = Formatter().vformat(values, [], self.keys)
    else:
      assert values is None
      if key in self.args:
        del self.args[key]
    return self

  def Append(self, key, values):
    if isinstance(values, list):
      assert all(isinstance(v, str) for v in values)
    elif isinstance(values, str):
      values = [values]
    else:
      assert values is None
      return self

    self.args[key] = _AsList(self.args.get(key))
    f = Formatter()
    formatted_values = [f.vformat(v, [], self.keys) for v in values]
    self.args[key].extend(formatted_values)
    return self

  def SubKeys(self, *keys):
    return dict((k, self.keys[k]) for k in keys)

  def ForEach(self, **kwargs):
    # {'a': [1,2,3], 'b': ['hi', 'hello']} ->
    # [[('a', 1), ('a', 2), ('a', 3)], [('b', 'hi'), ('b', 'hello')]] ->
    # [(('a', 1), ('b', 'hi')), (('a', 1), ('b', 'hello')), ...] ->
    # [{'a': 1, 'b': 'hi'}, {'a': 1, 'b': 'hello'}, {'a': 2, 'b': 'hi'}...]
    kvp_groups = [[(k, v) for v in _AsList(vs)] for k, vs in kwargs.iteritems()]
    new_objs = []
    for kvp in itertools.product(*kvp_groups):
      d = dict(kvp)
      new_objs.append(self.Expand(d))

    # Add new objects to context
    self.context._AddObjects(new_objs)
    self.context._RemoveObjects(self)
    return Select(self.context, new_objs)

  def Expand(self, keys):
    assert isinstance(keys, dict)
    assert all(isinstance(v, str) for v in keys.itervalues())
    result = self.__class__(self.context, keys)
    self._ExpandArgs(result, keys)
    self._ExpandTags(result, keys)
    return result

  def _ExpandArgs(self, o, keys):
    for k, vs in self.args.iteritems():
      o.Set(k, vs)

  def _ExpandTags(self, o, keys):
    f = Formatter()
    o.tags = set()
    for tag in self.tags:
      o.tags.add(f.vformat(tag, [], keys))

  def Tag(self, tags):
    if not isinstance(tags, list):
      tags = [tags]
    self.tags |= set(tags)
    return self


class Rule(Object):
  known_keys = ('name', 'command', 'description', 'depfile', 'generator',
      'restat', 'rspfile', 'rspfile_content')

  def Write(self, writer):
    assert self.name and self.command
    writer.rule(name=self.name,
                command=self.command,
                description=self.description,
                depfile=self.depfile,
                generator=self.generator,
                restat=self.restat,
                rspfile=self.rspfile,
                rspfile_content=self.rspfile_content)

  def GetUnknownVariables(self):
    return dict((k, v) for k, v in self.args.iteritems()
                if k not in Rule.known_keys)


class Build(Object):
  known_keys = ('outputs', 'rule', 'inputs', 'implicit', 'order_only')

  def Write(self, writer):
    writer.build(outputs=self.GetWriteAttr('outputs'),
                 rule=self.rule,
                 inputs=self.GetWriteAttr('inputs'),
                 implicit=self.GetWriteAttr('implicit'),
                 order_only=self.GetWriteAttr('order_only'),
                 variables=self._GetCombinedUnknownDicts())

  def GetUnknownVariables(self):
    return dict((k, v) for k, v in self.args.iteritems()
                if k not in Build.known_keys)

  def _CombineDicts(self, rule, build):
    combined = collections.defaultdict(list)
    for rk, rvs in rule.iteritems():
      combined[rk].extend(_AsList(rvs))
    for bk, bvs in build.iteritems():
      combined[bk].extend(_AsList(bvs))
    return combined

  def _GetCombinedUnknownDicts(self):
    rule = self.context._GetRule(self.rule)
    return self._CombineDicts(rule.GetUnknownVariables(),
                              self.GetUnknownVariables())

  def GetWriteAttr(self, attr):
    # Combine value from Rule and Build.
    rule = self.context._GetRule(self.rule)
    return _AsList(getattr(self, attr)) + _AsList(getattr(rule, attr))


class Variable(Object):
  def Write(self, writer):
    writer.variable(key=self.key, value=self.value)


class Filename(object):
  def __init__(self, fname):
    self.fname = fname

  @property
  def NoExtension(self):
    return os.path.splitext(self.fname)[0]

  @property
  def Extension(self):
    return os.path.splitext(self.fname)[1]


def _DictContainsDict(haystack, needle):
  for nk, nv in needle.iteritems():
    if nk not in haystack:
      return False
    if nv != haystack[nk]:
      return False
  return True


def _ListContainsList(haystack, needles):
  for needle in needles:
    if needle not in haystack:
      return False
  return True


def _SelectFrom(objs, tags, kwargs):
  result = []
  for o in objs:
    if o.tags & set(tags):
      result.append(o)
    else:
      for k, vs in kwargs.iteritems():
        try:
          ov = getattr(o, k)
        except AttributeError:
          continue
        if isinstance(ov, dict):
          assert isinstance(vs, dict)
          if _DictContainsDict(ov, vs):
            result.append(o)
        elif isinstance(ov, list):
          if isinstance(vs, list):
            if _ListContainsList(ov, vs):
              result.append(o)
          else:
            if vs in ov:
              result.append(o)
        elif isinstance(vs, list):
          if ov in vs:
              result.append(o)
        else:
          if ov == vs:
            result.append(o)
  return result


class Select(object):
  def __init__(self, context, objs):
    self.context = context
    self.objs = objs

  def And(self, *args, **kwargs):
    return Select(self.context, _SelectFrom(self.objs, args, kwargs))

  def __iter__(self):
    for o in self.objs:
      yield o

  def __getattr__(self, key):
    result = []
    for o in self.objs:
      v = getattr(o, key)
      if isinstance(v, list):
        result.extend(v)
      else:
        result.append(v)
    return result


class Context(object):
  def __init__(self):
    self.rules = {}
    self.objects = []

  def Rule(self, name, command, description=None, **kwargs):
    kwargs.update({
      'name': name,
      'command': command,
      'description': description
    })
    rule = Rule(self, **kwargs)
    self.objects.append(rule)
    self.rules[name] = rule
    return rule

  def Build(self, outputs, rule, inputs=None, implicit=None, order_only=None,
            **kwargs):
    kwargs.update({
      'outputs': outputs,
      'rule': rule,
      'inputs': inputs,
      'implicit': implicit,
      'order_only': order_only,
    })
    build = Build(self, **kwargs)
    self.objects.append(build)
    return build

  def Variable(self, key=None, value=None):
    var = Variable(self, **{'key': key, 'value': value})
    self.objects.append(var)
    return var

  def Select(self, *args, **kwargs):
    return Select(self, _SelectFrom(self.objects, args, kwargs))

  def _AddObjects(self, objs):
    assert isinstance(objs, list)
    assert all(isinstance(o, Object) for o in objs)
    self.objects.extend(objs)
    for o in objs:
      if isinstance(o, Rule):
        self.rules[o.name] = o

  def _RemoveObjects(self, objs):
    objs = _AsList(objs)
    assert all(isinstance(o, Object) for o in objs)
    for o in objs:
      try:
        self.objects.remove(o)
      except ValueError:
        pass
      if isinstance(o, Rule):
        del self.rules[o.name]

  def _GetRule(self, rulename):
    return self.rules[rulename]

  def _WriteAll(self, writer):
    for o in self.objects:
      o.Write(writer)


def ProcessFile(infname, outfname, defines):
  platform_dict = {
    'linux2': 'linux',
    'cygwin': 'win',
    'win32': 'win',
    'darwin': 'mac'
  }

  args = {}
  args['platform'] = platform_dict[sys.platform]
  for define in (defines or []):
    key, value = define.split('=')
    args[key] = value

  c = Context()
  gs = {
    'Args': args,
    'Build': c.Build,
    'Exclude': Exclude,
    'Filename': Filename,
    'GlobList': GlobList,
    'Prefix': Prefix,
    'Rule': c.Rule,
    'Select': c.Select,
    'Variable': c.Variable,
  }
  execfile(infname, gs, {})

  o = cStringIO.StringIO()
  c._WriteAll(ninja_syntax.Writer(o))

  with open(outfname, 'w') as outf:
    outf.write(o.getvalue())


def main(args):
  parser = optparse.OptionParser()
  parser.add_option('-D', '--define', action='append')
  parser.add_option('-o', dest='outfname')
  options, args = parser.parse_args()

  if len(args) > 1:
    parser.error('Too many files')
  if not options.outfname:
    parser.error('Expected output filename (-o)')

  infname = args[0]
  ProcessFile(infname, options.outfname, options.define)

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
