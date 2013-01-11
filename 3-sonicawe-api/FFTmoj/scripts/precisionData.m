%NRMSD = norm2(X-x)/sqrt(length(X)) = sqrt(mean(abs(X-x).^2))
%MAXERR = infnorm(X-x) = max(abs(X-x))
%RMSD = sqrt(mean(abs(X-x).^2))

function precisionData(lib)
	% libs = {"Ooura", "ClFft", "ClAmdFft", "CuFft"};
	RandomData = geth5("../data/RandomData.h5");
	PrecisionData = getPrecision(lib, RandomData);
	saveto = sprintf("../data/%sPrecision.dat", lib);
	save(saveto, "PrecisionData");
	end
end

function [precision] = getPrecision(lib, RandomData)
	more off
	disp(sprintf("Getting precision from %s", lib))
	sizes = sprintf("%sSizes", lib);
	sizefile = sprintf("../data/%s.dat", sizes);
	if exist(sizefile)
		load(sizefile);
		sizeid = 0;
		sizevar = eval(sizes)';
		sumsize = sum(sizevar);
		sumprog = 0;
		tic;
		for size = sizevar
			sizeid = sizeid+1;
			sumprog = sumprog + size;
			progress = sumprog / sumsize * 100;
			%sprintf("Lib: %s, Size: %i, ID: %i", lib, size, sizeid)
			result = geth5(sprintf("../data/%sResults%i.h5", lib, size));
			tempfft = fft(RandomData(1:size));
			format long;
			%disp(tempfft(1))
			%disp(result(1))
			%tempfft(1) = single(0);
			%result(1) = single(0);
			%tempfft(1) = tempfft(1)/length(tempfft);
			%result(1) = result(1)/length(result);
			%disp(tempfft(1))
			%disp(result(1))
			%error
			precision(sizeid, 1) = maxerr(tempfft(2:end), result(2:end));
			precision(sizeid, 2) = nrmsd(tempfft(2:end), result(2:end));
			timeleft = (toc/(progress/100))-toc;
			disp(sprintf("Done: %4i/%i (%3.1f%%, %im%.2is elapsed, %im%.2is remaining)", sizeid, columns(sizevar), progress, toc/60, mod(toc, 60), timeleft/60, mod(timeleft, 60)))
		end
	else
		sprintf("%s not found!\n", sizefile)
	end
end

function output = nrmsd(X, x)
	output = norm(X-x)/norm(X-mean(X));
end

function output = maxerr(X, x)
	output = max(abs(X-x));
end

function data = geth5(filename)
	data = sawe_loadstruct(filename);
	data = data.chunk;
	data = single(data);
end
