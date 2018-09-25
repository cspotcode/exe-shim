dotnet publish -c release -r win10-x64
# rm .\bin\release\netcoreapp2.0\win10-x64\docker-compose.exe
# mv .\bin\release\netcoreapp2.0\win10-x64\exe-shim.exe .\bin\release\netcoreapp2.0\win10-x64\docker-compose.exe 
cp ./exe-shim.config .\bin\release\netcoreapp2.0\win10-x64\
.\bin\release\netcoreapp2.0\win10-x64\exe-shim.exe