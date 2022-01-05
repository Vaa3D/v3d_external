source_dir=""
install_dir=""
cmake_current_binary_dir=""
x265_flags=""
extra_cflags=""
extra_ldflags=""
extra_libs=""
pkg_config_path=""
asm_exe=""

while [ "$1" != "" ]; do
    PARAM=`echo $1 | awk -F= '{print $1}'`
    VALUE=`echo $1 | awk -F= '{print $2}'`
    case $PARAM in
        --source_dir)
            source_dir=$VALUE
            ;;
        --install_dir)
            install_dir=$VALUE
            ;;
        --cmake_current_binary_dir)
            cmake_current_binary_dir=$VALUE
            ;;
        --x265_flags)
            x265_flags=$VALUE
            ;;
        --extra_cflags)
            extra_cflags=$VALUE
            ;;
        --extra_ldflags)
            extra_ldflags=$VALUE
            ;;
        --extra_libs)
            extra_libs=$VALUE
            ;;
        --pkg_config_path)
            pkg_config_path=$VALUE
            ;;
        --asm_exe)
            asm_exe=$VALUE
            ;;
        *)
            echo "ERROR: unknown parameter \"$PARAM\""
            exit 1
            ;;
    esac
    shift
done

export PKG_CONFIG_PATH=$pkg_config_path

mkdir -p $source_dir/../build/ffbuild
cd $source_dir/../build/ffbuild
$source_dir/configure --prefix=$install_dir \
            --x86asmexe=$asm_exe \
            --pkg-config=$cmake_current_binary_dir/pkg-config/install/bin/pkg-config \
            --disable-iconv \
            --disable-libxcb \
            --disable-static \
            --enable-shared \
            --enable-gpl \
            --enable-nonfree \
            --enable-version3 \
            --enable-runtime-cpudetect \
            $x265_flags \
            --extra-cflags="$extra_cflags" \
            --extra-ldflags="$extra_ldflags" \
            --extra-libs="$extra_libs"

make
make install
