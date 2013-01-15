function createPrecisionPlots
	libs = {"Ooura", "ClFft", "ClAmdFft"};
	
	colors = ['k', 'b', 'r', 'g'];
	%libs = {"ClAmdFft"};
	%libs = {"Ooura"};
	%libs = {"ClFft"};
	libid = 0;
	for lib = libs
		load(sprintf("data/%sPrecision.dat", lib{1}));
		sizes = sprintf("%sSizes", lib{1});
		sizefile = sprintf("data/%s.dat", sizes);
		load(sizefile);
		pd = [ eval(sizes) PrecisionData(:,1) PrecisionData(:,2) ];
		if (strcmp(lib, "Ooura"))
			OouraData = pd;
		elseif (strcmp(lib, "ClAmdFft"))
			ClAmdFftData = pd;
		elseif (strcmp(lib, "ClFft"))
			ClFftData = pd;
		else
			error;
		endif
		libid = libid + 1;
		%linestring(libid) = sprintf("%c;%s;", colors(libid), libs(libid){1});

	end
	
	powersof2 = {'2^8', '2^9', '2^{10}', '2^{11}', '2^{12}', '2^{13}', '2^{14}', '2^{15}', '2^{16}', '2^{17}', '2^{18}', '2^{19}', '2^{20}', '2^{21}', '2^{22}'};
	
	figure;
	loglog(OouraData(:,1), OouraData(:,2), "k;Ooura;", ClFftData(:,1), ClFftData(:,2), "b;ClFft;", ClAmdFftData(:,1), ClAmdFftData(:,2), "r;ClAmdFft;");
	title("Maxerr");
	ylabel("Maximum Error", "fontsize", 16);
	xlabel("FFT size", "fontsize", 16);
	set (gca, 'xtick', OouraData(:,1));
	set (gca, 'xticklabel', powersof2);
	legend("boxon");
	print("Maxerr.png", "-dpng");
	figure;
	loglog(OouraData(:,1), OouraData(:,3), "k;Ooura;", ClFftData(:,1), ClFftData(:,3), "b;ClFft;", ClAmdFftData(:,1), ClAmdFftData(:,3), "r;ClAmdFft;");
	title("NRMSD");
	ylabel("NRMSD", "fontsize", 16);
	xlabel("FFT size", "fontsize", 16);
	set (gca, 'xtick', OouraData(:,1));
	set (gca, 'xticklabel', powersof2);
	legend("boxon");
	print("NRMSD.png", "-dpng");
end