flags = [
'-Wall',
'-Wextra',
'-Wc++98-compat',
'-D__KERNEL__',
'-DUSE_CLANG_COMPLETER',
'-std=c99',
'-x', 'c',
'-I', '.',
'-I./module/include',
'-I./library/include',
'-I/lib/modules/5.7.10-arch1-1/build/include',
'-I/lib/modules/5.7.10-arch1-1/build/arch/x86/include'
]

def Settings( **kwargs ):
    return {
            'flags': flags,
    }

