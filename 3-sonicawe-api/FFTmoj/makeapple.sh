qmake CONFIG+=useopenclamdapple 
cd ../../../../src
qmake CONFIG+=useopenclamdapple CONFIG+=testlib
cd ../lib/gpumisc
qmake CONFIG+=useopenclamdapple
