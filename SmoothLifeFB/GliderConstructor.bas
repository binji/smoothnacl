' SmoothLife glider constructor
'
'  esc    quit
'  space  toggle between running the simulation and drawing the glider
'  w/s    in drawing mode: in/decrease speed


Const pi = 6.2831853

Const SX = 1000	' screen resolution
Const SY = 660

Const TX = 80		' texture size
Const TY = 80

Const BX = 500		' snm resolution
Const BY = 500

Const BYL = (10+BY)/8+2

Const ri = 4
Const ra = 12
Const bi = 1
Const ba = 1
Const gi = pi/2*ri^2
Const ga = pi/2*ra^2-gi
Const rra = ra+ba

Dim Shared As Double tex(TX,TY),ca(TX,TY),cb(TX,TY)
Dim Shared As Double snm(BX,BY), scc(BX,BY)
Dim Shared As Integer v

Declare Sub mymain
mymain
End


' draw the desired glider shape, symmetry is achieved automatically
' dial the desired glider speed and then press space to calc snm and test run it
'
Private Sub drawstuff ()
	Dim As Integer mx, my, buttons
	Dim As Integer kx, px, py
	Dim As Double  col
	Dim As String i

	kx = 0
	Do
		GetMouse mx, my,,buttons

		If buttons Then
			If buttons And 1 Then col = 1
			If buttons And 2 Then col = 0
			px = Int(mx/4)
			py = Int(my/4)
			If px>=1 And px<=TX And py>=1 And py<=TY Then
				tex(px,py)=col
				Line(px*4,py*4)-(px*4+3,py*4+3),RGB(col*255,col*255,col*255),bf

				px = TX+1-px
				tex(px,py)=col
				Line(px*4,py*4)-(px*4+3,py*4+3),RGB(col*255,col*255,col*255),bf
			EndIf
		EndIf

		i = Inkey$

		If i=Chr(27) Then End
		If i=" " Then Exit Do
		If i="w" Then v += 1
		If i="s" Then v -= 1

		Locate BYL+2,70:Print "v=";v

		kx += 1
		If kx=100 Then ScreenCopy:kx=0
	Loop
End Sub


' calculate snm
'
Private Sub makesnm ()
	Dim As Integer px, py, dx, dy, kx, ky
	Dim As Double n, m, l, f
	Dim As Integer scn

	' initialize snm to 0.5 and collision counts to 0
	For py = 1 To BY
		For px = 1 To BX
			snm(px,py) = 0.5
			scc(px,py) = 0
		Next
	Next
	scn = 0

	' determine snm and collision counts with tex shifted by v
	For py = 1 To TY
		For px = 1 To TX
			n = 0
			m = 0
			For dx = -rra To rra
				kx = px+dx
				If kx<1 Then kx += TX
				If kx>TX Then kx -= TX
				For dy = -rra To rra
					ky = py+dy
					If ky<1 Then ky += TY
					If ky>TY Then ky -= TY

					l = Sqr(dx*dx+dy*dy)
					f = tex(kx,ky)
					If l<=ri-bi/2 Then
						m += f
					ElseIf l<=ri+bi/2 Then
						m += (ri+bi/2-l)/bi*f
						n += (l-ri+bi/2)/bi*f
					ElseIf l<=ra-ba/2 Then
						n += f
					ElseIf l<=ra+ba/2 Then
						n += (ra+ba/2-l)/ba*f
					End If
				Next
			Next
			m /= gi
			n /= ga

			dx = Int(n*BX+0.5)+1
			dy = Int(m*BY+0.5)+1
			kx = px
			If kx<1 Then kx += TX
			If kx>TX Then kx -= TX
			ky = py+v
			If ky<1 Then ky += TY
			If ky>TY Then ky -= TY
			If snm(dx,dy)<>0.5 And snm(dx,dy)<>tex(kx,ky) Then
				scc(dx,dy) += 1
				scn += 1
			EndIf
			snm(dx,dy) = tex(kx,ky)
		Next
	Next

	' display snm (where defined) and collisions (in red)
	For kx = 1 To BX
		For ky = 1 To BY
			f = snm(kx,ky)
			PSet(TX*4+100+kx,10+ky),RGB(f*255,f*255,f*255)
			f = scc(kx,ky)
			If f>1 Then f=1
			If f>0 Then PSet(TX*4+100+kx,10+ky),RGB(f*255,0,0)
		Next
	Next
	Locate BYL+2,70:Print "v=";v;"  scn=";scn;"     "
End Sub


' run it and see the glider in action (or see it dissolve)
'
Private Sub runit ()
	Dim As Integer px, py, dx, dy, kx, ky
	Dim As Double n, m, l, f
	Dim As String i

	For py = 1 To TY
		For px = 1 To TX
			ca(px,py) = tex(px,py)
		Next
	Next


	Locate BYL+4,70:Print "running..."
	Do

		For py = 1 To TY
			For px = 1 To TX
				n = 0
				m = 0
				For dx = -rra To rra
					kx = px+dx
					If kx<1 Then kx += TX
					If kx>TX Then kx -= TX
					For dy = -rra To rra
						ky = py+dy
						If ky<1 Then ky += TY
						If ky>TY Then ky -= TY

						l = Sqr(dx*dx+dy*dy)
						f = ca(kx,ky)
						If l<=ri-bi/2 Then
							m += f
						ElseIf l<=ri+bi/2 Then
							m += (ri+bi/2-l)/bi*f
							n += (l-ri+bi/2)/bi*f
						ElseIf l<=ra-ba/2 Then
							n += f
						ElseIf l<=ra+ba/2 Then
							n += (ra+ba/2-l)/ba*f
						End If
					Next
				Next
				m /= gi
				n /= ga

				dx = Int(n*BX+0.5)+1
				dy = Int(m*BY+0.5)+1
				f = snm(dx,dy)
				cb(px,py) = f
				Line(px*4,4*(TY+2)+py*4)-(px*4+3,4*(TY+2)+py*4+3),RGB(f*255,f*255,f*255),bf
			Next
		Next

		For py = 1 To TY
			For px = 1 To TX
				ca(px,py) = cb(px,py)
			Next
		Next

		i = Inkey$

		If i=" " Then Exit Do

		ScreenCopy

	Loop
	Locate BYL+4,70:Print "             "
End Sub


' main
'
Sub mymain
	Dim As Integer px, py
	
	ScreenRes SX,SY,32,2
	ScreenSet 1,0

	Randomize Timer

	Line(0,0)-(SX,SY),RGB(70,70,70),bf

	For px = 1 To TX
		For py = 1 To TY
			Line(px*4,py*4)-(px*4+3,py*4+3),RGB(0,0,0),bf
		Next
	Next

	Do
		drawstuff ()
		makesnm ()
		runit ()
	Loop

End Sub
