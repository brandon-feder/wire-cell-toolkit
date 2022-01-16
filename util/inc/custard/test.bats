#!/usr/bin/env bats

dump () {
    f="$1" ; shift
    ext="${f##*.}"
    if [ "$ext" = "npy" ] ; then
        python -c 'import sys, numpy; a=numpy.load(sys.argv[1]); print(a.dtype); print(a)' $f
    else
        cat $f
    fi
}

diff_two () {
    one="$1" ; shift
    two="$1" ; shift

    ext="${one##*.}"
    echo "EXT $ext"

    echo "$one $(dump $one)"
    echo "$two $(dump $two)"

    md5sum $one $two
    diff -u <(dump $one) <(dump $two)
}


# make a working directory for the test exec and set some vars
do_prep () {
    name="$1" ; shift    
    extra="$1"
    texe=$(realpath bin/$name)    
    outd=test/${name}_${extra}
    rm -rf $outd
    mkdir -p $outd
    outd=$(realpath $outd)
    echo $texe
    echo $outd
}

# many tests have same write pattern: $text foo.tar file ...
do_write () {
    name="$1" ; shift
    comp="$1" ; shift
    ext="${1:-hpp}"             # foder file type

    do_prep $name write_$comp

    srcdir=$(pwd)
    cd $outd

    if [ "$ext" = "npy" ] ; then
        cd $outd/
        $srcdir/gen.py
        cd -
    else
        cp $srcdir/*.${ext} .
    fi
    mkdir -p cus gnu

    echo "write test for $name in $(pwd)"
    ls -l

    echo run $texe cus.tar *.${ext}
    run $texe cus.tar *.${ext}
    echo "$output"
    [ "$status" -eq 0 ]
    [ -s cus.tar ]
    ls -lR
    [ $(stat -c %s cus.tar) -gt 1024 ]

    # check we can untar and get back original files
    echo "untar archive we just made"
    tar -C cus -xvf cus.tar

    touch *.$ext                # freshen since our tests use "now"
    # check we can make nearly identical tar file with GNU
    tar -b2 -cf gnu-full.tar *.${ext}
    # our tar files do not pad as much GNU
    dd if=gnu-full.tar of=gnu.tar bs=1 count=$(stat -c %s cus.tar)

    ls -l cus

    for one in *.${ext}
    do
        diff_two $one cus/$one
    done


    od -a cus.tar > cus.od
    od -a gnu.tar > gnu.od
    diff -u cus.od gnu.od 

    if [ "$comp" = "no" ] ; then
        return
    fi

    # now check gzip compression.
    run $texe   cus.tar.gz *.${ext}
    echo "$output"
    [ "$status" -eq 0 ]
    [ -s cus.tar.gz ]
    rm cus/*
    tar -C cus -xzf cus.tar.gz
    ls -l cus/
    for one in *.${ext}
    do
        diff_two $one cus/$one
    done

    # now check bzip2 compression.
    run $texe   cus.tar.bz2 *.${ext}
    echo "$output"
    [ "$status" -eq 0 ]
    [ -s cus.tar.bz2 ]
    rm cus/*
    tar -C cus -xjf cus.tar.bz2
    ls -l cus/
    for one in *.${ext}
    do
        diff_two $one cus/$one
    done


    date > okay
}


do_read () {
    name="$1" ; shift
    comp="$1" ; shift
    ext="${1:-hpp}"             # fodder extension
    do_prep $name read_$comp

    srcdir=$(pwd)
    cd $outd/

    mkdir orig
    cd orig
    if [ "$ext" = "npy" ] ; then
        $srcdir/gen.py
    else
        cp $srcdir/*.$ext .
    fi
    tar -cf ../gnu.tar *.${ext}
    if [ "$comp" = "yes" ] ; then
        tar -czf ../gnu.tar.gz *.${ext}
        tar -cjf ../gnu.tar.bz2 *.${ext}
    fi
    cd ..
    ls -lR
    pwd
    echo $texe

    tars=(gnu.tar)
    if [ "$comp" = "yes" ] ; then
        tars=(gnu.tar gnu.tar.bz2 gnu.tar.gz)
    fi

    for one in ${tars[*]}
    do
        echo $one
        rm -f *.${ext}
        pwd
        echo $texe $one
        run $texe $one
        echo "$output"
        [ "$status" -eq 0 ]
        for n in *.${ext}
        do
            ls -l $n orig/$n
            md5sum $n orig/$n
            diff -u $n orig/$n
        done
    done
    date > okay
}

@test "test_custard write" {
    do_write test_custard no hpp
}

@test "test_custard read" {
    do_read test_custard no hpp
}

@test "test_custard_boost write" {
    do_write test_custard_boost yes hpp
}

@test "test_custard_boost read" {
    do_read test_custard_boost yes hpp
}


@test "test_custard_boost_pigenc write" {
    do_write test_custard_boost_pigenc yes npy
}

@test "test_custard_boost_pigenc read" {
    do_read test_custard_boost_pigenc yes npy
}
