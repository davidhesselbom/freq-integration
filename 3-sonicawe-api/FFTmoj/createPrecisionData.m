function createPrecisionData
	cd scripts;
	%libs = {"Ooura", "ClFft", "ClAmdFft", "CuFft"};
	%libs = {"ClAmdFft"};
	libs = {"Ooura"};
	%libs = {"Clfft"};
	%libs = {"CuFft"};
	for lib = libs
		precisionData(lib{1})
	end
	cd ..
end