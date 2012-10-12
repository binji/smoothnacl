' SmoothLife in FreeBasic
'
' todo
'	- all the functions that have outer x/y loops, i.e.
'		untangle, tangle, fft2d_stage, kernelmul, snm and the copybuffer fns
'		could be made parallel and multicore with ThreadCreate/ThreadWait
'


Const pi = 6.283185307179586477

Const BX = 8     ' number of bits of the buffer size
Const BY = 8

Const NX = 2^BX   ' size of real buffer
Const NY = 2^BY

Const b1 = 0.278			' SmoothLife birth/death values
Const b2 = 0.365
Const d1 = 0.267
Const d2 = 0.445
Const alphan = 0.028		' alphas for the smooth steps
Const alpham = 0.147
Const CRA = 15				' outer radius of kernel

Type complex
	r As Double
	i As Double
End Type

Const AA = 0		' array
Const KR = 1		' kernel ring
Const KD = 2		' kernel disk
Const AN = 3		' n
Const AM = 4		' m
Const ARB = 5		' number of real buffers

Const AF = 0		' array Fourier transformed
Const KRF = 1		' kernel ring Fourier
Const KDF = 2		' kernel disk Fourier
Const ANF = 3		' n Fourier
Const AMF = 4		' m Fourier
Const FFT0 = 5		' intermediate buffers for the stages
Const FFT1 = 6		' (toggle between them)
Const AFB = 7		' number of Fourier buffers

Dim Shared As Double rr(NX,NY,ARB)		' real arrays
Dim Shared As complex a(NX/2+1,NY,AFB)	' complex arrays
Dim Shared As Double kflr, kfld			' areas of kernels (ring, disk)
Dim Shared As Double px1(NX/2+1,BX,2), px3(NX/2+1,BX,2)	' plans for fft
Dim Shared As complex px2(NX/2+1,BX,2)
Dim Shared As Double py1(NY,BY,2), py3(NY,BY,2)
Dim Shared As complex py2(NY,BY,2)

Declare Sub main
main
End


Private Function cmplx (r As Double, i As Double) As complex
	Dim As complex c
	c.r = r
	c.i = i
	Return c
End Function


Private Function cscl (a As complex, s As Double) As complex
	Dim As complex c
	c.r = s*a.r
	c.i = s*a.i
	Return c
End Function


Private Function cmul (a As complex, b As complex) As complex
	Dim As complex c
	c.r = a.r*b.r - a.i*b.i
	c.i = a.r*b.i + a.i*b.r
	Return c
End Function


Private Function cadd (a As complex, b As complex) As complex
	Dim As complex c
	c.r = a.r + b.r
	c.i = a.i + b.i
	Return c
End Function


Private Function csub (a As complex, b As complex) As complex
	Dim As complex c
	c.r = a.r - b.r
	c.i = a.i - b.i
	Return c
End Function


Private Function cexp (a As Double) As complex
	Dim As complex c
	c.r = Cos(a)
	c.i = Sin(a)
	Return c
End Function


Private Function ccnj (a As complex) As complex
	Dim As complex c
	c.r = a.r
	c.i = -a.i
	Return c
End Function


' draw real buffer on the whole screen
' (if the screen resolution is 1024x768)
'
Private Sub drawscr (m As Integer)
	Dim As Integer x, y
	Dim As Integer Ptr pt
	
	pt = ScreenPtr
	For y = 0 To 768-1
	For x = 0 To 1024-1
		*pt = RGB(rr(x,y,m)*255,0,0)
		pt += 1
	Next
	Next
End Sub


' draw real buffer
'
Private Sub drawrr (m As Integer, qx As Integer, qy As Integer)
	Dim As Integer x, y
	Dim As Double r, i

	For y = 0 To NY-1
	For x = 0 To NX-1
		r = rr(x,y,m)
		PSet(qx+x,qy+y),RGB(r*255,0,0)
	Next
	Next
End Sub


' initialize real buffer with random boxes
'
Private Sub initrr (m As Integer)
	Dim As Integer x, y, t, sx, sy, b, h, c

	For t = 0 To (NX/CRA)*(NY/CRA)
		sx = Int(Rnd*NX)
		sy = Int(Rnd*NY)
		b = CRA+Int(Rnd*CRA)
		h = CRA+Int(Rnd*CRA)
		c = Int(Rnd*2)
		For x = sx To sx+b
		For y = sy To sy+h
			If x<NX And y<NY Then rr(x,y,m) = c
		Next
		Next
	Next
End Sub


' make both kernels, ring and disk, i.e. draw them in their (real) buffers
'
Private Sub makekernelrr (er As Integer, ed As integer)
	Dim As Integer x, y, sx, sy, px, py
	Dim As Double l, n, m, b, ri, ra
	
	ra = CRA
	ri = ra/3
	b = 1

	kflr = 0
	kfld = 0
	For x = 0 To NX-1
	For y = 0 To NY-1
		sx = x-NX/2
		sy = y-NY/2
		l = Sqr(sx^2+sy^2)
		If l<ri-b/2 Then
			n = 0
			m = 1
		ElseIf l<ri+b/2 Then
			n = (l-ri+b/2)/b
			m = (ri+b/2-l)/b
		ElseIf l<ra-b/2 Then
			n = 1
			m = 0
		ElseIf l<ra+b/2 Then
			n = (ra+b/2-l)/b
			m = 0
		Else
			n = 0
			m = 0
		EndIf
		
		px = x+NX/2
		If px>NX-1 Then px-=NX
		If px<0 Then px+=NX
		py = y+NY/2
		If py>NY-1 Then py-=NY
		If py<0 Then py+=NY
		
		rr(px,py,er) = n
		rr(px,py,ed) = m
		kflr += n
		kfld += m
	Next
	Next
End Sub


' do the kernel multiply that achieves the convolution
' (this is in packed complex format)
'
Private Sub kernelmul (vo As Integer, ke As Integer, na As Integer, sc As double)
	Dim As Integer x, y
	Dim As complex b, c

	For x = 0 To NX/2
	For y = 0 To NY-1
		a(x,y,na) = cmul (a(x,y,vo), cscl(a(x,y,ke),sc))
	Next
	Next
End Sub


' copy a real buffer into a complex (Fourier) one
' (half real half imaginary part)
'
Private Sub copybufferrc (vo As Integer, na As Integer)
	Dim As Integer x, y

	For x = 0 To NX/2-1
	For y = 0 To NY-1
		a(x,y,na).r = rr(2*x+0,y,vo)
		a(x,y,na).i = rr(2*x+1,y,vo)
	Next
	Next
End Sub


' copy a complex buffer back into a real one
'
Private Sub copybuffercr (vo As Integer, na As Integer)
	Dim As Integer x, y

	For x = 0 To NX/2-1
	For y = 0 To NY-1
		rr(2*x+0,y,na) = a(x,y,vo).r
		rr(2*x+1,y,na) = a(x,y,vo).i
	Next
	Next
End Sub


Private Function bitreverse (m As Integer, b As Integer) As Integer
	Dim As Integer c, t

	c = 0
	For t = 0 To b-1
		c = (c Shl 1) Or ((m Shr t) And 1)
	Next
	Return c
End Function


Private Sub fft2d_plany (eb As Integer, si As integer)
	Dim As Integer l, j, y, s
	Dim As Double w
	
	s = (si+1)/2
	For y = 0 To NY-1

		l = 2^eb
		j = y Mod l
		If j<l/2 Then
			If eb=1 Then
				py1(y,eb,s) = bitreverse(y    ,BY)
				py3(y,eb,s) = bitreverse(y+l/2,BY)
			Else
				py1(y,eb,s) = y
				py3(y,eb,s) = y+l/2
			EndIf
			w = si*pi*j/l
			py2(y,eb,s).r = Cos(w)
			py2(y,eb,s).i = Sin(w)
		Else
			If eb=1 Then
				py1(y,eb,s) = bitreverse(y-l/2,BY)
				py3(y,eb,s) = bitreverse(y    ,BY)
			Else
				py1(y,eb,s) = y-l/2
				py3(y,eb,s) = y
			EndIf
			w = si*pi*(j-l/2)/l
			py2(y,eb,s).r = -Cos(w)
			py2(y,eb,s).i = -Sin(w)
		EndIf

	Next
End Sub


Private Sub fft2d_planx (eb As Integer, si As integer)
	Dim As Integer l, j, x, s
	Dim As Double w
	
	s = (si+1)/2
	For x = 0 To NX/2
		If si=1 And eb=0 Then

			px1(x,eb,s) = x
			px3(x,eb,s) = NX/2-x
			px2(x,eb,s) = cexp(si*pi*(x/NX+1/4))

		ElseIf si=-1 And eb=BX-1+1 Then

			If x=0 Or x=NX/2 Then
				px1(x,eb,s) = 0
				px3(x,eb,s) = 0
			Else
				px1(x,eb,s) = x
				px3(x,eb,s) = NX/2-x
			EndIf
			px2(x,eb,s) = cexp(si*pi*(x/NX+1/4))

		ElseIf x<NX/2 Then

			l = 2^eb
			j = x Mod l
			If j<l/2 Then
				If eb=1 Then
					px1(x,eb,s) = bitreverse(x    ,BX-1)
					px3(x,eb,s) = bitreverse(x+l/2,BX-1)
				Else
					px1(x,eb,s) = x
					px3(x,eb,s) = x+l/2
				EndIf
				w = si*pi*j/l
				px2(x,eb,s).r = Cos(w)
				px2(x,eb,s).i = Sin(w)
			Else
				If eb=1 Then
					px1(x,eb,s) = bitreverse(x-l/2,BX-1)
					px3(x,eb,s) = bitreverse(x    ,BX-1)
				Else
					px1(x,eb,s) = x-l/2
					px3(x,eb,s) = x
				EndIf
				w = si*pi*(j-l/2)/l
				px2(x,eb,s).r = -Cos(w)
				px2(x,eb,s).i = -Sin(w)
			EndIf

		EndIf
	Next
End Sub


Private Sub fft2d_stage (d As Integer, eb As Integer, si As Integer, vo As Integer, na As Integer)
	Dim As Integer x, y, s
	Dim As complex b, c

	s = (si+1)/2
	For x = 0 To NX/2
	For y = 0 To NY-1

		If d=1 Then

			If si=1 And eb=0 Then
				
				b = a(px1(x,eb,s),y,vo)
				c = ccnj(a(px3(x,eb,s),y,vo))
				b = cadd(cadd(b,c),cmul(csub(b,c),px2(x,eb,s)))
				a(x,y,na) = cscl(b,0.5*Sqr(2))
			
			ElseIf si=-1 And eb=BX-1+1 Then

				b = a(px1(x,eb,s),y,vo)
				c = ccnj(a(px3(x,eb,s),y,vo))
				b = cadd(cadd(b,c),cmul(csub(b,c),px2(x,eb,s)))
				a(x,y,na) = cscl(b,0.5/Sqr(2))
			
			ElseIf x<NX/2 Then

				b = cadd (a(px1(x,eb,s),y,vo), cmul (px2(x,eb,s), a(px3(x,eb,s),y,vo)))
				a(x,y,na) = cscl(b,1/Sqr(2))
			
			EndIf
		
		ElseIf d=2 Then

			b = cadd (a(x,py1(y,eb,s),vo), cmul (py2(y,eb,s), a(x,py3(y,eb,s),vo)))
			a(x,y,na) = cscl(b,1/Sqr(2))
		
		EndIf

	Next
	Next
	
End Sub


Private Sub fft2drr (vo As Integer, na As Integer, si As Integer)
	Dim As Integer t, fftcurrent, fftother

	fftcurrent = FFT0
	fftother = FFT1

	If si=-1 Then
		
		copybufferrc (vo, fftcurrent)
		For t = 1 To BX-1+1
			fft2d_stage (1, t, si, fftcurrent, fftother)
			Swap fftcurrent, fftother
		Next
		For t = 1 To BY
			If t=BY Then
				fft2d_stage (2, t, si, fftcurrent, na)
			Else
				fft2d_stage (2, t, si, fftcurrent, fftother)
			EndIf
			Swap fftcurrent, fftother
		Next

	Else

		For t = 1 To BY
			If t=1 Then
				fft2d_stage (2, t, si, vo, fftother)
			Else
				fft2d_stage (2, t, si, fftcurrent, fftother)		
			EndIf
			Swap fftcurrent, fftother
		Next
		For t = 0 To BX-1
			fft2d_stage (1, t, si, fftcurrent, fftother)
			Swap fftcurrent, fftother
		Next
		copybuffercr (fftcurrent, na)
	
	EndIf

End Sub


' here starts the SmoothLife part

' various forms of step functions to build the snm function

Function func_hard (x As Double, a As Double) As double
	If x<a Then Return 0:Else Return 1
End Function

Function func_linear (x As Double, a As Double, ea As Double) As double
	If x<a-ea/2 Then
		Return 0
	ElseIf x>a+ea/2 Then
		Return 1
	Else
		Return (x-a)/ea+0.5
	EndIf
End Function

Function func_hermite (x As Double, a As Double, ea As Double) As Double
	If x<a-ea/2 Then
		Return 0
	ElseIf x>a+ea/2 Then
		Return 1
	Else
		Dim As Double m
		m = (x-(a-ea/2))/ea
		Return m*m*(3 - 2*m)
	EndIf
End Function

Function func_sin (x As Double, a As Double, ea As Double) As double
	If x<a-ea/2 Then
		Return 0
	ElseIf x>a+ea/2 Then
		Return 1
	Else
		Return Sin(pi/2*(x-a)/ea)*0.5+0.5
	EndIf
End Function

Function func_smooth (x As Double, a As Double, ea As Double) As double
	Return 1/(1+exp(-(x-a)*4/ea))
End Function

' choose here how the step function should look like
'
Private Function sigmoid_a (x As Double, a As Double, ea As Double) As Double
	'sigmoid_a = func_hard (x, a)
	'sigmoid_a = func_linear (x, a, ea)
	'sigmoid_a = func_hermite (x, a, ea)
	'sigmoid_a = func_sin (x, a, ea)
	sigmoid_a = func_smooth (x, a, ea)
End Function

Private Function sigmoid_b (x As Double, b As Double, eb As double) As Double
	sigmoid_b = 1-sigmoid_a (x, b, eb)
End Function

Private Function sigmoid_ab (x As Double, a As Double, b As Double) As Double
	sigmoid_ab = sigmoid_a (x, a, alphan)*sigmoid_b (x, b, alphan)
End Function

Private Function sigmoid_mix (x As Double, y As Double, m As Double) As Double
	sigmoid_mix = x*(1-func_smooth (m, 0.5, alpham)) + y*func_smooth (m, 0.5, alpham)
End Function

' apply snm to n and m, this is the time step
'
Private Sub snm (en As Integer, em As Integer, ej As Integer)
	Dim As Integer x, y
	Dim As Double n, m
	
	For x = 0 To NX-1
	For y = 0 To NY-1
		n = rr(x,y,en)
		m = rr(x,y,em)
		rr(x,y,ej) = sigmoid_ab (n, sigmoid_mix (b1, d1, m), sigmoid_mix (b2, d2, m))
	Next
	Next
End Sub


Sub main ()
	Dim As String i
	Dim As Integer t

	If NX>=1024 And NY>=1024 Then
		Screen 20,32,2,1
	Else
		Screen 19,32,2
	EndIf

	Randomize Timer

	For t = 1 To BY
		fft2d_plany (t, -1)
		fft2d_plany (t, 1)
	Next
	For t = 0 To BX-1+1
		fft2d_planx (t, -1)
		fft2d_planx (t, 1)
	Next
	
	makekernelrr (KR, KD)
	fft2drr (KR, KRF, -1)
	fft2drr (KD, KDF, -1)

	initrr (AA)
	
	ScreenSet 1,0

	Do
		Cls
		If NX>=1024 And NY>=1024 Then
			drawscr (AA)
		Else
			drawrr (AA, 30, 30)
		EndIf

		Dim As Double tim1 = Timer
	
		fft2drr (AA, AF, -1)
		kernelmul (AF, KRF, ANF, Sqr(NX*NY)/kflr)
		kernelmul (AF, KDF, AMF, Sqr(NX*NY)/kfld)
		fft2drr (ANF, AN, 1)
		fft2drr (AMF, AM, 1)
		snm (AN, AM, AA)

		Locate 0,0:Print Using "###.###";Timer-tim1
	
		ScreenCopy
		i = InKey$
		
		If i="b" Then initrr(AA)
		
	Loop While i<>Chr(27)
End Sub
