x = msgbox("Are you sure you want to remove PM33XX ?" ,4, "PM33XX Confirmation")

If x = vbNo Then Wscript.Quit(0)

If x = VbYes Then

Set WshShell = CreateObject("WScript.Shell")

X = WshShell.run("cmd /c MsiExec.exe ""/I {C35C5578-1A99-4815-AD34-E6E20A01F6F2}""",0, true)

Const HKEY_CURRENT_USER = &H80000001

strComputer = "."

Set objReg = GetObject("winmgmts:\\" & _
    strComputer & "\root\default:StdRegProv")
strKeyPath = "SOFTWARE\TEC\PM33XX"

objReg.DeleteKey HKEY_CURRENT_USER, strKeyPath

End If

