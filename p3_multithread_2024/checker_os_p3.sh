#!/bin/bash
#set -x

declare -i nota=0
declare -i incremento=1


# ****************************************


testeo(){
   #$1 nombre del fichero test
   echo "File: " $1 
   echo "Nprod: " $2
   echo "MCons: " $3 
   echo "Buffsize: " $4
   #echo "expected file out: " $4
   ./store_manager $1 $2 $3 $4 > out_calc  2>/dev/null
   if diff -i -w -q --ignore-all-space $5 out_calc > /dev/null; then
    echo " OK"
    cat out_calc
    echo -n "1 " >> excel
    nota=$(( nota + incremento )) 
    echo $nota 
   else
    echo " Error"
    echo -n "0 " >> excel
    echo " === test === "
         cat  $1
    echo " === output ./store_manager  === "
    cat  out_calc
    echo " === expected output  === " 
    cat  $5
   fi
   rm -f out* 
   echo
   echo
}


testeoError(){
   #$1 nombre del fichero test
   echo "File: " $1 
   echo "Nprod: " $2
   echo "MCons: " $3 
   echo "Buffsize: " $4
   #echo "expected file out: " $5
   eval ./store_manager $1 $2 $3 $4 > /dev/null 2> /dev/null  
   if [ $? -eq 0 ]; 
   then 
         echo " Wrong returned value"
   echo -n "0 " >> excel 
   else 
         echo " OK"
         echo -n "1 " >> excel
         nota=$(( nota + incremento )) 
   echo $nota 
   fi
   echo
   echo
    
}

testeoErrorParams(){
   #$1 nombre del fichero test
   echo "File: " $1 
   echo "Nprod: " $2
   echo "MCons: " $3 
   echo "Buffsize: " $4
   echo "Extra arg: " $5
   eval ./store_manager $1 $2 $3 $4 $5> /dev/null 2> /dev/null  
   if [ $? -eq 0 ]; 
   then 
         echo " Wrong returned value"
   echo -n "0 " >> excel 
   else 
         echo " OK"
         echo -n "1 " >> excel
         nota=$(( nota + incremento ))
   echo $nota  
   fi
   echo
   echo
    
}


#*****************************************


if [ -z "$1" ]
  then
    echo "No argument supplied"
    exit
fi

clear

echo "*** TESTING P3 "

 
TDIR="./testdir"
mkdir $TDIR
cp $1 $TDIR

echo "File : $1"

#cp Makefile $TDIR
#cp *.c $TDIR
#cp *.h $TDIR
#cp authors.txt $TDIR
#cp *.test $TDIR

cd $TDIR

text="50
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7"
echo -e "$text" > 50-50.test

text2="200
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7"
echo -e "$text2" > 50-200.test

text3="50
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7"
echo -e "$text3" > 200-50.test

text4="200
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7
1 PURCHASE 100
2 PURCHASE 55
3 PURCHASE 30
4 PURCHASE 20
5 PURCHASE 10
1 SALE 5
1 SALE 10
3 SALE 2
4 SALE 1
4 SALE 5
5 SALE 3
2 SALE 15
1 SALE 22
3 SALE 10
5 SALE 5
1 SALE 13
2 SALE 7
3 SALE 2
1 SALE 33
2 SALE 1
5 SALE 1
4 SALE 5
3 SALE 6
2 SALE 2
1 SALE 7"
echo -e "$text4" > 200-200.test

unzip $1
touch empty.test
touch no_perm.test
chmod -r no_perm.test
chmod -w no_perm.test


if [ ! -f authors.txt ]; then
   echo "Error: missing authors.txt file"
   cd ..
   rm -r $TDIR
   exit
fi


make clean 2> /dev/null > /dev/null
make       2> /dev/null > /dev/null


echo "Compiling"
if [ ! -f store_manager ]; then
   echo "Error: does not compile"
   cd ..
#  rm -r $TDIR
   exit
else
   echo "OK"
fi


# BUILDING TESTS
text="Total: 120 euros
Stock:
  Product 1: 20
  Product 2: 60
  Product 3: 20
  Product 4: 18
  Product 5: 2"
echo -e "$text" > 50-50.res

text="Total: 120 euros
Stock:
  Product 1: 20
  Product 2: 60
  Product 3: 20
  Product 4: 18
  Product 5: 2"
echo -e "$text" > 200-50.res

text="Total: 480 euros
Stock:
  Product 1: 80
  Product 2: 240
  Product 3: 80
  Product 4: 72
  Product 5: 8"
echo -e "$text" > 200-200.res

# ERROR TESTS

testeoErrorParams 50-50.test 1 2 3 4
testeoError 50-50.test 0 1
testeoError 50-50.test 1 0
testeoError 50-50.test -1 1
testeoError 50-50.test 1 -1

# TESTS WITH FILES

testeo 50-50.test 3 2 10 50-50.res
testeo 200-50.test 3 2 10 200-50.res
testeo 200-200.test 3 2 10 200-200.res
testeoError 50-200.test 3 10

# TESTING CONCURRENCY WITH DIFFERENT VALUES OF NPROD AND BUFFSIZE

testeo 200-200.test 1 1 10 200-200.res
testeo 200-200.test 1 1 200 200-200.res
testeo 200-200.test 3 3 10 200-200.res
testeo 200-200.test 4 4 50 200-200.res
testeo 200-200.test 5 5 50 200-200.res
testeo 200-200.test 6 6 200 200-200.res
testeo 200-200.test 4 4 200 200-200.res
testeo 200-200.test 5 5 20 200-200.res
testeo 200-200.test 6 6 10 200-200.res

# GRADE & DELETE FILES

fnota=$((echo scale=2 ; echo $nota*10 / 18) | bc )
echo "Grade: $fnota" 
echo $fnota > ../nota.txt

cp authors.txt ..
cd ..

rm -rf testdir 