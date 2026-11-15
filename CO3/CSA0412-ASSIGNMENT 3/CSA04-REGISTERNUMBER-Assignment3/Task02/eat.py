m, t = 100, 20          # memory access 100 ns, TLB lookup 20 ns
print(f"Assumptions: memory access m = {m} ns, TLB lookup t = {t} ns, single-level page table")
print(f"Without TLB: EAT = 2m = {2*m} ns")
print(f"{'Hit ratio':>10} {'EAT (ns)':>10} {'Slow-down vs 100 ns':>22}")
for h in (0.80, 0.95, 0.99):
    e = h*(t+m) + (1-h)*(t+2*m)
    print(f"{h*100:>9.0f}% {e:>10.1f} {e/m:>21.2f}x")
print()
for bits in (32, 48):
    n = 2**(bits-12); b = 4 if bits==32 else 8
    sz = n*b
    print(f"{bits}-bit, 4 KB pages: entries = 2^{bits-12} = {n:,}; PTE = {b} B; flat table = {sz:,} B = {sz/2**20:,.0f} MB = {sz/2**30:,.1f} GB")
