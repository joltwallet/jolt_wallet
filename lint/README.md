# checkpatch.pl from the Linux Kernel

This git repo includes a script to download `checkpatch.pl` script from [`torvalds/linux/scripts`](https://github.com/torvalds/linux/tree/master/scripts).


## Quick Install

On macOS, do this:

``` sh
brew tap riboseinc/checkpatch
brew install --HEAD checkpatch
```


## Usage


### `git-checkpatch`

`git-checkpatch` is a quick way of using `checkpatch.pl` outside the kernel tree.

```sh
git-checkpatch HEAD
```

=>

```
7667e7a (HEAD -> mybranch, origin/mybranch) My Commit Message
total: 0 errors, 0 warnings, 20 lines checked

Your patch has no obvious style problems and is ready for submission.

NOTE: Ignored message types: ARRAY_SIZE COMMIT_LOG_LONG_LINE COMPLEX_MACRO EXECUTE_PERMISSIONS FILE_PATH_CHANGES LONG_LINE LONG_LINE_COMMENT LONG_LINE_STRING MISSING_SIGN_OFF MULTISTATEMENT_MACRO_USE_DO_WHILE NAKED_SSCANF NEW_TYPEDEFS RETURN_PARENTHESES SPLIT_STRING SSCANF_TO_KSTRTO STATIC_CONST_CHAR_ARRAY STORAGE_CLASS SYMBOLIC_PERMS TRAILING_SEMICOLON USE_FUNC
```

For your convenience, these `checkpatch.pl` checks checks are *IGNORED* when running
`git-checkpatch`:

```
COMPLEX_MACRO
TRAILING_SEMICOLON
LONG_LINE
LONG_LINE_STRING
LONG_LINE_COMMENT
SYMBOLIC_PERMS
NEW_TYPEDEFS
SPLIT_STRING
USE_FUNC
COMMIT_LOG_LONG_LINE
FILE_PATH_CHANGES
MISSING_SIGN_OFF
RETURN_PARENTHESES
STATIC_CONST_CHAR_ARRAY
ARRAY_SIZE
NAKED_SSCANF
SSCANF_TO_KSTRTO
EXECUTE_PERMISSIONS
MULTISTATEMENT_MACRO_USE_DO_WHILE
STORAGE_CLASS
```

### Normal `checkpatch.pl`

The normal `checkpatch.pl` can be invoked like this outside the kernel tree:

```sh
checkpatch.pl --no-tree
```


## Download

To download `checkpatch.pl`, run the following command:

```sh
./download.sh
```
