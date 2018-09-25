$ErrorActionPreference = 'Stop'

$oldPath = $env:PATH
$oldPathExt = $env:PATHEXT
$oldPwd = $PWD
try {
    $where = get-command where.exe
    $env:PATH = "$PSScriptRoot\path1;$PSScriptRoot\path2;C:\Windows\System32"
    $env:PATHEXT=".BFIRST;.ASECOND;.EXE"
    Set-Location $PSScriptRoot
    
    function reset() {
        rm -r ./path1/*
        rm -r ./path2/*
        rm -r ./cwd/*
        $env:PATH = "$PSScriptRoot\path1;$PSScriptRoot\path2;C:\Windows\System32"
        $env:PATHEXT=".BFIRST;.ASECOND;.EXE"
        Set-Location $PSScriptRoot
    }
    
    function touch([parameter(valuefrompipeline)]$path) {
        '' | out-file $path
    }
    
    set-alias where _where -option allscope -force
    function _where($name) {
        try {
            push-location $PSScriptRoot/cwd
            (where.exe $name | select-object -first 1).replace("$PSScriptRoot\",'')
        } finally {
            pop-location
        }
    }
    describe 'where.exe' {
        BeforeEach {
            reset
        }
        
        it 'finds CWD before PATH' {
            touch ./cwd/foo.asecond
            touch ./path1/foo.asecond
            touch ./path2/foo.asecond
            where foo | should -be cwd\foo.asecond
        }
        
        it 'finds first PATH entry before second' {
            touch ./path1/foo.asecond
            touch ./path2/foo.bfirst
            where foo | should -be path1\foo.asecond
        }
        
        it 'finds first extension before second' {
            touch ./path1/foo.asecond
            touch ./path1/foo.bfirst
            where.exe foo | write-host
            where foo | should -be path1\foo.bfirst
            # THIS IS WRONG where.exe lists foo.asecond but cmd.exe tries to invoke foo.bfirst
        }
        it 'extensionless or matching PATHEXT?' {
            touch ./path1/foo
            touch ./path1/foo.bfirst
            cmd | write-host
        }
        
        # TODO try to run foo.bbb, but foo.bbb.aaa exists in path
        
    }

} finally {
    $env:PATH = $oldPath
    $env:PATHEXT = $oldPathExt
    Set-Location $oldPwd
}

