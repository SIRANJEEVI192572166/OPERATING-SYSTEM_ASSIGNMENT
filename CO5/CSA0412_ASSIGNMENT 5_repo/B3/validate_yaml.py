import sys, yaml
print("$ python3 validate_yaml.py " + " ".join(sys.argv[1:]))
for f in sys.argv[1:]:
    for d in yaml.safe_load_all(open(f)):
        k = d["kind"]; n = d["metadata"]["name"]
        if k == "Deployment":
            s = d["spec"]; c = s["template"]["spec"]["containers"][0]
            assert s["selector"]["matchLabels"] == s["template"]["metadata"]["labels"]
            print(f"{f}: {k}/{n} replicas={s['replicas']} selector={s['selector']['matchLabels']}")
            print("   requests:", c["resources"]["requests"], " limits:", c["resources"]["limits"])
        elif k == "Service":
            p = d["spec"]["ports"][0]
            print(f"{f}: {k}/{n} type={d['spec']['type']} port={p['port']}->{p['targetPort']}")
        else:
            sp = d["spec"]
            print(f"{f}: {k}/{n} min={sp['minReplicas']} max={sp['maxReplicas']} cpu-target={sp['metrics'][0]['resource']['target']['averageUtilization']}%")
print("YAML syntax and structure: OK (no cluster available in this sandbox)")
