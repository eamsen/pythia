import sys

class Entity:
  def __init__(self, name, rank, score, answer_type):
    self.name = name
    self.rank = rank
    self.score = score
    self.answer = answer_type

  def __str__(self):
    return self.name + ":" + str(self.answer)


def parse_entity(raw):
  return Entity(raw[1], int(raw[0]), float(raw[2]), int(raw[3]))


class Query:
  def __init__(self, num, select_r, select_p, select_app_r, select_app_p):
    self.num = num
    self.select_r = select_r
    self.select_p = select_p
    self.select_app_r = select_app_r
    self.select_app_p = select_app_p
    self.entities = []

  def __str__(self):
    return str(self.num) + ": " + str(map(str, self.entities))

def parse_query(raw):
  query = Query(int(raw[0]), float(raw[1]), float(raw[2]), float(raw[3]), float(raw[4]))
  for i in range((len(raw) - 5) / 4):
    query.entities.append(parse_entity(raw[i * 4 + 5:(i+1) * 4 + 5]))
  return query

def parse_log(path):
  with open(path) as f:
    raw = f.read().split(";");
    queries = []
    for r in raw:
      queries.append(parse_query(r.split(",")))
    return queries;


def main():
  queries = parse_log(sys.argv[1]);
  print "\n".join(map(str, queries))



if __name__ == "__main__":
  main()
