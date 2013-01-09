function createPrecisionData
	cd scripts;
	libs = {"Ooura", "ClFft", "ClAmdFft", "CuFft"};
	%libs = {"Ooura"};
	for lib = libs
		precisionData(lib{1})
	end
	cd ..
end