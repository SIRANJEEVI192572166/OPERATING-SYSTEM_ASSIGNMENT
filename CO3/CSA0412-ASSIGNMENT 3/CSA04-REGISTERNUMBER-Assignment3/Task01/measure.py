import subprocess, time
def run(cmd, n=2000):
    t=time.perf_counter()
    for _ in range(n): subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return (time.perf_counter()-t)/n*1000
for name in (['./bank_static'],['./bank_dyn'],['./bank_dl','nocrypto'],['./bank_dl']):
    print(f"{' '.join(name):22s} average start-up + run time = {run(name):.3f} ms (2000 runs)")
