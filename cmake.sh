#!/bin/bash


# echo -e "\e[1;36m \e[0m"
#\ E [1; 36 set color to cyan \ E [0 reset color back.
#First of all, I know that echo printf is printed on the terminal in the shell. Generally, 
# echo is commonly used;
#Note: font color reset = 0,
#Black = 30,
#Red = 31,
#Green = 32,
# ***=33
#Blue = 34, magenta = 35, cyan = 36, white = 37
#Background color reset = 0, black = 40, red = 41, green = 42,
#* * * = 43, blue = 44, magenta = 45, cyan = 46, white = 47
#To print color text, enter the following command:
# echo -e "\e[1;36m \e[0m"
#The copy code is as follows:


echo "if you don't know how to use:'./1.sh help'"

##The presence of cmake & & make in the option will cause shell ambiguity
case $* in
"cmake")
    # mkdir ./build
    if [ ! -d "./build/" ]
    then 
        echo -e "\e[1;32m mkdir ./build \e[0m"
        mkdir ./build
    fi

    # mkdir ./bin
    if [ ! -d "./bin" ]
    then 
        echo -e "\e[1;32m mkdir bin \e[0m"
        mkdir ./bin
    fi

    # cd ./build/ [and] rm ./* -r [and] cmake .. && make
    echo -e "\e[1;32m cd ./build \e[0m"
    cd ./build || exit
    echo -e "\e[1;31m rm ./* -r \e[0m"
    rm -r ./*
    echo -e "\e[1;32m cmake .. && make \e[0m"
    cmake -DSDK=on .. && make
    cmake -DAPP=on .. && make
    cmake -DCDCS=on .. && make
    cmake -DDEAMON=on .. && make
    echo -e "\e[1;32m exit build \e[0m"
    cd .. || exit 
    echo -e "\e[1;36m finished \e[0m"
    ;;
"clean")
    # if [ !-d "./lib/" ]
    # then
    #     echo "no lib dir"
    #     exit
    # fi
    # rm ./lib/*

    if [ ! -d "./build/" ]
    then 
        echo "no dir build, exit"
        exit
    fi
    cd ./build || exit
    echo "cd build & rm ./* -rf"
    rm ./* -rf
    cd .. || exit 

    if [ ! -d "./bin/" ]
    then 
        echo "no dir bin, exit"
        exit
    fi
    cd ./bin || exit
    echo "cd bin & rm ./*.log -r"
    rm ./*.log 
    ;;
"check")
    if [ ! -f "./bin/zlog.conf" ]
    then
        echo "no ./bin/zlog.conf file"
        exit
    fi
    zlog-chk-conf ./bin/zlog.conf
    ;;


"help")
    echo "usage"
    echo "'cmake.sh cmake' to build bin"
    echo "'cmake.sh clean' to clean data"
    ;;
"start")
    ./bin/sdk_deamon start
    ;;   
"stop")
    ./bin/sdk_deamon stop
    ;;  
esac
