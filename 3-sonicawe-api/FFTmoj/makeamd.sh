qmake CONFIG+=useopenclamdamd
cd ../../../../src
qmake CONFIG+=useopenclamdamd CONFIG+=testlib
cd ../lib/gpumisc
qmake CONFIG+=useopenclamd
