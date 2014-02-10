%%
fs = 44100;
L = 2;
T = 0:1/fs:L;
f0 = fs/5;
number = 1;

t0 = T.*(1+0.101*sin(2*pi*T*number));
y = sin(2*pi*f0*t0);

hz0 = 1000;
n = floor(fs/L/hz0)-1;
N = floor(length(T)/n);
for k=1:n
    hz = hz0*k;
    p = (1:N)+(k-1)*N;
    y0 = sin(2*pi*hz*T(p));
    y0 = conv(y0,ones(N,1)/sqrt(N),'same');
    y(p) = y(p) + y0;
end

y = y - mean(y);
y = y / max(abs(y));
audiowrite('testwav.wav', y, fs);

plot(T,t0)
