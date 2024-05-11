if [ ! -d "build" ];then
    mkdir build
else
    echo "directory build is exist, now start build."
fi

cd build
cmake ../
make