#!/bin/sh

for ll_file in $LL_FILES_DIR/*.ll ;do
    echo "####### $ll_file #######"
    printf 'command line: "%s" "%s" "%s"\n' "$BUILD_DIR"/main "$ll_file" "$FLAGS"
    "$BUILD_DIR"/main "$ll_file" "$FLAGS" || {
        status=$?
        echo "****** benchmark CRASHED: $ll_file *******"
        printf 'command line: "%s" "%s" "%s"\n' "$BUILD_DIR"/main "$ll_file" "$FLAGS"
        exit $status
    }
done
