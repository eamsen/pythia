while True:
  line = raw_input().split()
  c = int(line[0])
  w = line[1]
  if c > 100 and len(w) > 2 and len(w) < 200 and w.isalpha():
    print "%s\t%i" % (w, c)
