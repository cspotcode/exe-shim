Setup command prompt:

```powershell
pushd 'C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build'
./vcvarsall.bat x86
popd
```

Run build:

```powershell
cl ./main.c
# to specify subsystem (shouldn't be necessary)
cl ./main.c /link /subsystem:console
```
