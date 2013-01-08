function createPrecisionData
	cd scripts;
	libs = {"Ooura", "ClFft", "ClAmdFft", "CuFft"};
	for lib = libs
		precisionData(lib{1})
	end
	cd ..
end