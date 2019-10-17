﻿; toggle left click
RShift::
Toggle := !Toggle
If (Toggle)
	{
		Send {Click Down Left}
	} Else {
		Send {Click Up Left}
	}
	Return

; virtual finger
Home:: 
Toggle := !Toggle
If (Toggle)
	{
		Send {Click Down Right}
	} Else {
		Send {Click Up Right}
	}
	Return

; split
End:: 
Toggle := !Toggle
If (Toggle)
	{
		Send {Click Left}{s down}{Click Down Right}}
	} Else {
		Send {Click Up Right}{s up}
	}
	Return

; delete
PgUp:: 
Toggle := !Toggle
If (Toggle)
	{
		Send {Click Left}{d down}{Click Down Right}}
	} Else {
		Send {Click Up Right}{d up}
	}
	Return
	
; retype
PgDn:: 
Toggle := !Toggle
If (Toggle)
	{
		Send {Click Left}{r down}{Click Down Right}}
	} Else {
		Send {Click Up Right}{r up}
	}
	Return

; join
`::
Toggle := !Toggle
If (Toggle)
	{
		Send {Click Left}{j down}}
	} Else {
		Send {j up}
	}
	Return