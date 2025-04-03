#!/bin/bash

cd $(dirname $0)
PROJECT_PATH=$(pwd)

BUILD_OPTIONS="-g2 -O2 $BUILD_OPTIONS"
INCLUDE_PATHS="$PROJECT_PATH/inc-c"

if [[ $CC = "" ]];
then
    CC="gcc"
fi

find_sources() {
    ((LEN_PREFIX=${#PROJECT_PATH} + 8))
    find $PROJECT_PATH/src-c -type f | grep .c\$ | cut -c $LEN_PREFIX-
}

make_object_name() {
    ((LEN_SOURCE_NAME_WITHOUT_SUFFIX=${#1}-2))
    echo $(echo $1 | tr "-" "_" | tr "/" "-" | cut -c "-$LEN_SOURCE_NAME_WITHOUT_SUFFIX").o
}

build_sources() {
    mkdir -p $PROJECT_PATH/.build/objects
    rm -f $PROJECT_PATH/.build/objects/*.o

    SOURCE_FILES=$(find_sources)
    for SOURCE_FILE in $SOURCE_FILES
    do
        echo "==> $SOURCE_FILE"
        OBJECT_NAME=$(make_object_name $SOURCE_FILE)
        $CC $BUILD_OPTIONS \
            -c -o .build/objects/$OBJECT_NAME \
            $PROJECT_PATH/src-c/$SOURCE_FILE \
            -I $INCLUDE_PATHS
    done
}

make_static_library() {
    ar -rvs $PROJECT_PATH/libcini.a \
        $PROJECT_PATH/.build/objects/*.o
}

case $1 in
    "" | "build")
        build_sources
        make_static_library
        ;;
    *)
        echo "Unknown Action!"
        ;;
esac

