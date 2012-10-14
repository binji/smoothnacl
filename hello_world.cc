/* Copyright (c) 2012 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define _USE_MATH_DEFINES 1
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ppapi/c/pp_stdint.h"
#include "ppapi/c/pp_completion_callback.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/c/pp_graphics_3d.h"
#include "ppapi/c/pp_module.h"
#include "ppapi/c/pp_var.h"
#include "ppapi/c/ppb.h"
#include "ppapi/c/ppb_core.h"
#include "ppapi/c/ppb_graphics_3d.h"
#include "ppapi/c/ppb_instance.h"
#include "ppapi/c/ppb_messaging.h"
#include "ppapi/c/ppb_opengles2.h"
#include "ppapi/c/ppb_var.h"
#include "ppapi/c/ppp.h"
#include "ppapi/c/ppp_instance.h"
#include "ppapi/c/ppp_messaging.h"
#include "ppapi/c/ppb_url_loader.h"
#include "ppapi/c/ppb_url_request_info.h"

#include "ppapi/c/ppp_graphics_3d.h"
#include "ppapi/lib/gl/gles2/gl2ext_ppapi.h"

#include <GLES2/gl2.h>
#include "matrix.h"
#include "smoothlife.h"

static PPB_Messaging* ppb_messaging_interface = NULL;
static PPB_Var* ppb_var_interface = NULL;
static PPB_Core* ppb_core_interface = NULL;
static PPB_Graphics3D* ppb_g3d_interface = NULL;
static PPB_Instance* ppb_instance_interface = NULL;
static PPB_URLRequestInfo* ppb_urlrequestinfo_interface = NULL;
static PPB_URLLoader* ppb_urlloader_interface = NULL;

static PP_Instance g_instance;
static PP_Resource g_context;

const int kNumResources = 2;
const char* g_toLoad[kNumResources] = {
  "2D/snm.vert",
  "2D/snm.frag",
};
const char* g_loadedData[kNumResources];
int g_LoadCnt = 0;

GLuint g_snm_prog;


void PostMessage(const char *fmt, ...);
char* LoadFile(const char *fileName);

void InitGL();
void InitProgram();
void Render();


static struct PP_Var CStrToVar(const char* str) {
  if (ppb_var_interface != NULL) {
    return ppb_var_interface->VarFromUtf8(str, strlen(str));
  }
  return PP_MakeUndefined();
}


void PostMessage(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  char msg[4096];
  vsnprintf(msg, sizeof(msg), fmt, args);

  if (ppb_messaging_interface)
    ppb_messaging_interface->PostMessage(g_instance, CStrToVar(msg));

  va_end(args);
}

void MainLoop(void* foo, int bar) {
  if (g_LoadCnt == kNumResources) {
    printf("initializing...\n");
    InitProgram();
    g_LoadCnt++;
  }
  if (g_LoadCnt > kNumResources) {
    Render();
    if (!glGetError()) {
      PP_CompletionCallback cc = PP_MakeCompletionCallback(MainLoop, 0);
      ppb_g3d_interface->SwapBuffers(g_context, cc);
    }
  } else {
    PP_CompletionCallback cc = PP_MakeCompletionCallback(MainLoop, 0);
    ppb_core_interface->CallOnMainThread(0, cc, 0);
  }
}

void InitGL() {
  int32_t attribs[] = {
    PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 8,
    PP_GRAPHICS3DATTRIB_SAMPLES, 0,
    PP_GRAPHICS3DATTRIB_SAMPLE_BUFFERS, 0,
    PP_GRAPHICS3DATTRIB_WIDTH, 640,
    PP_GRAPHICS3DATTRIB_HEIGHT, 480,
    PP_GRAPHICS3DATTRIB_NONE
  };

  g_context =  ppb_g3d_interface->Create(g_instance, 0, attribs);
  int32_t success =  ppb_instance_interface->BindGraphics(g_instance, g_context);
  if (success == PP_FALSE) {
    glSetCurrentContextPPAPI(0);
    printf("Failed to set context.\n");
    return;
  }
  glSetCurrentContextPPAPI(g_context);

  glViewport(0, 0, 640, 480);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}


void InitProgram() {
  glSetCurrentContextPPAPI(g_context);

  InitializeVbo();
  InitializeTextures();
  g_snm_prog = MakeProgram(g_loadedData[0], g_loadedData[1]);
  GetSNMLocations(g_snm_prog);
  if (glGetError()) {
    printf("Error initializing program!\n");
  }
}


void Render() {
  glClearColor(0.5, 0.5, 0.5, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  makesnm(g_snm_prog, AN, AM, AA);
}


typedef void (*OpenCB)(void *dataPtr);
struct OpenRequest {
  PP_Resource loader_;
  PP_Resource request_;
  char* buf_;
  void* data_;
  int64_t size_;
  int64_t avail_;
  OpenCB cb_;
};


void FreeRequest(OpenRequest* req) {
  if (req) {
    ppb_core_interface->ReleaseResource(req->request_);
    ppb_core_interface->ReleaseResource(req->loader_);
    free(req);
  }
}


static void URLPartialRead(void* user_data, int mode) {
  OpenRequest* req = (OpenRequest *) user_data;
  int64_t total;
  int32_t cnt;

  if (mode < 0) {
    free(req->buf_);
    req->cb_(NULL);
    FreeRequest(req);
    return;
  }

  req->avail_ += mode;
  total = req->size_ - req->avail_;

  cnt = (total > LONG_MAX) ? LONG_MAX : (int32_t) total;
  // If we still have more to do, re-issue the read.
  if (cnt > 0) {
    int32_t bytes = ppb_urlloader_interface->ReadResponseBody(req->loader_,
        (void *) &req->buf_[req->avail_], cnt,
        PP_MakeCompletionCallback(URLPartialRead, req));

    // If the reissue completes immediately, then process it.
    if (bytes != PP_OK_COMPLETIONPENDING) {
      URLPartialRead(user_data, bytes);
    }
    return;
  } 
  
  // Nothing left, so signal complete.
  req->cb_(req);
  FreeRequest(req);
  printf("Loaded\n");
}


static void URLOpened(void* user_data, int mode) {
  OpenRequest* req = (OpenRequest *) user_data;

  int64_t cur, total;
  int32_t cnt;
  ppb_urlloader_interface->GetDownloadProgress(req->loader_, &cur, &total);

  // If we can't preallocate the buffer because the size is unknown, then
  // fail the load.
  if (total == -1) {
    req->cb_(NULL);
    FreeRequest(req);
    return;
  }

  // Otherwise allocate a buffer with enough space for a terminating
  // NUL in case we need one.
  cnt = (total > LONG_MAX) ? LONG_MAX : (int32_t) total;
  req->buf_ = (char *) malloc(cnt + 1);
  req->buf_[cnt] = 0;
  req->size_ = cnt;
  int32_t bytes = ppb_urlloader_interface->ReadResponseBody(req->loader_, req->buf_, cnt,
      PP_MakeCompletionCallback(URLPartialRead, req));

  // Usually we are pending.
  if (bytes == PP_OK_COMPLETIONPENDING) return;

  // But if we did complete the read, then dispatch the handler.
  URLPartialRead(req, bytes);
}

void LoadURL(PP_Instance inst, const char *url, OpenCB cb, void *data) {
  OpenRequest* req = (OpenRequest*) malloc(sizeof(OpenRequest));
  memset(req, 0, sizeof(OpenRequest));

  req->loader_ = ppb_urlloader_interface->Create(inst);
  req->request_ = ppb_urlrequestinfo_interface->Create(inst);
  req->cb_ = cb;
  req->data_ = data;

  if (!req->loader_ || !req->request_) {
    cb(NULL);
    FreeRequest(req);
    return;
  }

  ppb_urlrequestinfo_interface->SetProperty(req->request_, 
      PP_URLREQUESTPROPERTY_URL, CStrToVar(url));
  ppb_urlrequestinfo_interface->SetProperty(req->request_,
      PP_URLREQUESTPROPERTY_METHOD, CStrToVar("GET"));
  ppb_urlrequestinfo_interface->SetProperty(req->request_,
      PP_URLREQUESTPROPERTY_RECORDDOWNLOADPROGRESS, PP_MakeBool(PP_TRUE));

  int val = ppb_urlloader_interface->Open(req->loader_, req->request_, 
      PP_MakeCompletionCallback(URLOpened, req));

  if (val != PP_OK_COMPLETIONPENDING) {
    cb(NULL);
    free(req);
  }
}

void Loaded(void* data) {
  OpenRequest *req = (OpenRequest *) data;
  if (req && req->buf_) {
    char **pptr = (char **) req->data_;
    *pptr = req->buf_;
    g_LoadCnt++;
    return;
  }
  PostMessage("Failed to load asset.\n");
}


static PP_Bool Instance_DidCreate(PP_Instance instance,
                                  uint32_t argc,
                                  const char* argn[],
                                  const char* argv[]) {
  g_instance = instance;
  for (int i = 0; i < kNumResources; ++i)
    LoadURL(instance, g_toLoad[i], Loaded, &g_loadedData[i]);
  return PP_TRUE;
}

static void Instance_DidDestroy(PP_Instance instance) {
  for (int i = 0; i < kNumResources; ++i)
    delete[] g_loadedData[i];
}

static void Instance_DidChangeView(PP_Instance instance,
                                   PP_Resource view_resource) {
  if (g_context == 0) {
    InitGL();
    MainLoop(NULL, 0);
  }
}

static void Instance_DidChangeFocus(PP_Instance instance,
                                    PP_Bool has_focus) {
}

static PP_Bool Instance_HandleDocumentLoad(PP_Instance instance,
                                           PP_Resource url_loader) {
  /* NaCl modules do not need to handle the document load function. */
  return PP_FALSE;
}


PP_EXPORT int32_t PPP_InitializeModule(PP_Module a_module_id,
                                       PPB_GetInterface get_browser) {
  ppb_core_interface = (PPB_Core*)(get_browser(PPB_CORE_INTERFACE));
  ppb_instance_interface = (PPB_Instance*)get_browser(PPB_INSTANCE_INTERFACE);
  ppb_messaging_interface =
      (PPB_Messaging*)(get_browser(PPB_MESSAGING_INTERFACE));
  ppb_var_interface = (PPB_Var*)(get_browser(PPB_VAR_INTERFACE));
  ppb_urlloader_interface =
      (PPB_URLLoader*)(get_browser(PPB_URLLOADER_INTERFACE));
  ppb_urlrequestinfo_interface =
      (PPB_URLRequestInfo*)(get_browser(PPB_URLREQUESTINFO_INTERFACE));
  ppb_g3d_interface = (PPB_Graphics3D*)get_browser(PPB_GRAPHICS_3D_INTERFACE);
  if (!glInitializePPAPI(get_browser))
    return PP_ERROR_FAILED;
  return PP_OK;
}


PP_EXPORT const void* PPP_GetInterface(const char* interface_name) {
  if (strcmp(interface_name, PPP_INSTANCE_INTERFACE) == 0) {
    static PPP_Instance instance_interface = {
      &Instance_DidCreate,
      &Instance_DidDestroy,
      &Instance_DidChangeView,
      &Instance_DidChangeFocus,
      &Instance_HandleDocumentLoad,
    };
    return &instance_interface;
  }
  return NULL;
}


PP_EXPORT void PPP_ShutdownModule() {
}
