#!/bin/sh

for ll_file in $LL_FILES;do
    echo "####### $ll_file #######"
    "$BUILD_DIR"/main "$ll_file" "$FLAGS" || {
        status=$?
        echo "****** benchmark CRASHED: $ll_file *******"
        exit $status
    }
done
