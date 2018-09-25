# Pros

Fixes all cmd-related argv-encoding bugs.
npm-installed tools can be invoked using the same APIs as regular .exes.  (normal process spawning)
This eliminates the complexity of invoking `cmd /C` and the extra argv encoding that it entails.
Correct process termination behavior.  (Fixes https://github.com/npm/npm/issues/2938)

# Cons

Does not provide "bug-for-bug" compatibility.
External code that works-around the escaping behavior of .cmd will need to change.
- can be partially mitigated by leaving the .cmd shim, adding the .exe.

# Notes

With Powershell slowly being encouraged on new Windows systems, .cmd's no longer offer the benefit of running in-process.  Unix already spawns a proxy-process via the bash shims.

# IMPLEMENTATION NOTES

Get path to self
- GetModuleFileName: https://msdn.microsoft.com/en-us/library/ms683197(VS.85).aspx
Replace extension with .exeshimini or .ini
Read interpreter name and target name from .ini
- GetPrivateProfileString: https://msdn.microsoft.com/en-us/library/ms724353.aspx

If interpreter:
    Check for <interpreterName>.exe in same directory as shim.

    Otherwise:
    Manually implement PATH and PATHEXT lookup of interpreter.
    - TODO does cmd.exe look in current directory as well?
    - cmd-shim looks for ./node.exe first; does it do this for non-node interpreters?
    - https://msdn.microsoft.com/en-us/library/windows/desktop/hh707082(v=vs.85).aspx

    Once we find a matching path, no need to search for alternatives.

Copy positional args verbatim from the CommandLine string.
- TODO how to skip past first argv?
If interpreter specified, prepend target to CommandLine

Attempt direct process invocation
- TODO piping stdio?
- TODO allocate console?
  - Since we're replacing cmd, we should already be a console subsystem app, so no need.
  - We have no intention of hiding the console, so we can let Windows auto-allocate one.
- TODO handle inheritance?  Since we're replacing cmd, this may not be necessary.  We only need to emulate cmd.exe

If process invocation fails:
Call FindExecutable to get .exe interpreter associated with file
prepend it to CommandLine
invoke it
- TODO allocate a new console?

Last resort: ShellExecuteEx
- why is this last resort?  because it doesn't allow stdio redirection or blocking.
- https://msdn.microsoft.com/en-us/library/windows/desktop/bb762154(v=vs.85).aspx

# Powershell's implementation of native command invocation:
https://github.com/PowerShell/PowerShell/blob/fe3e44f3055ccd57e19ce1d29a5320e2f3891705/src/System.Management.Automation/engine/NativeCommandProcessor.cs