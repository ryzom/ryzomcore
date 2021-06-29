
with open("sitem_list.txt", "w") as w:
	with open("sitem_list_r2.txt", "r") as r:
		for l in r:
			w.write(l)
	with open("sitem_list_wk.txt", "r") as r:
		for l in r:
			if len(l.strip()) > 0 and not l.startswith(";"):
				w.write(l)
