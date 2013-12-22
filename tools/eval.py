import sys

class Entity:
  def __init__(self, name, rank, score, answer_type):
    self.name = name
    self.rank = rank
    self.score = score
    self.answer = answer_type

  def __str__(self):
    return "[%s, %i, %f, %i]" % (self.name, self.rank, self.score, self.answer)


def parse_entity(raw):
  return Entity(raw[1], int(raw[0]), float(raw[2]), int(raw[3]))


class Query:
  def __init__(self, num, num_answers, select_r, select_p, select_app_r, select_app_p):
    self.num = num
    self.num_answers = num_answers
    self.select_r = select_r
    self.select_p = select_p
    self.select_app_r = select_app_r
    self.select_app_p = select_app_p
    self.entities = []

  def __str__(self):
    s = str(self.num) + ": ["
    for e in self.entities:
      s += str(e) + ","
    return s

def parse_query(raw):
  query = Query(int(raw[0]), int(raw[1]), float(raw[2]), float(raw[3]), float(raw[4]), float(raw[5]))
  for i in range((len(raw) - 6) / 4):
    query.entities.append(parse_entity(raw[i * 4 + 6:(i+1) * 4 + 6]))
  return query

def parse_log(path):
  with open(path) as f:
    raw = f.read().split(";");
    queries = []
    for r in raw:
      queries.append(parse_query(r.split(",")))
    return queries;

def f_score(recall, prec):
  if recall == 0.0 and prec == 0.0:
    return 0.0
  return 2.0 * recall * prec / (recall + prec)

def find_max_precision(query):
  num_answers = query.num_answers
  best_r = 0.0
  best_app_r = 0.0
  best_p = 0.0
  best_app_p = 0.0
  best_f = 0.0
  best_app_f = 0.0
  found = 0.0
  app_found = 0.0
  for i, e in enumerate(query.entities):
    found += e.answer == 1
    app_found += e.answer > 0

    recall = found / num_answers
    app_recall = app_found / num_answers

    prec = found / (i + 1)
    app_prec = app_found / (i + 1)

    best_p = max(best_p, prec)
    best_app_p = max(best_app_p, app_prec)

    best_r = max(best_r, recall)
    best_app_r = max(best_app_r, app_recall)

    best_f = max(best_f, f_score(recall, prec))
    best_app_f = max(best_app_f, f_score(app_recall, app_prec)) 

  return best_r, best_app_r, best_p, best_app_p, best_f, best_app_f


def main():
  epsilon = 0.0000001
  queries = parse_log(sys.argv[1]);
  # print "\n".join(map(str, queries))
  print "id\t recall\t\t prec\t\t f"
  avg_f_ratio = [0.0, 0.0]
  for q in queries:
    best = find_max_precision(q)
    f_scores = (f_score(q.select_r, q.select_p),
                f_score(q.select_app_r, q.select_app_p))
    f_ratio = [0.0, 0.0]
    if best[4] < epsilon:
      f_ratio[0] = 1.0
    else:
      f_ratio[0] = f_scores[0] / best[4]
    if best[5] < epsilon:
      f_ratio[1] = 1.0
    else:
      f_ratio[1] = f_scores[1] / best[5]
    avg_f_ratio[0] += f_ratio[0]
    avg_f_ratio[1] += f_ratio[1]

    print "%i s\t %.2f/%.2f\t %.2f/%.2f\t %.2f/%.2f" %\
      (q.num, q.select_r, q.select_app_r, q.select_p, q.select_app_p,\
       f_scores[0], f_scores[1])
    print "%i o\t %.2f/%.2f\t %.2f/%.2f\t %.2f/%.2f\t %.2f/%.2f" %\
        (q.num, best[0], best[1], best[2], best[3], best[4], best[5],
         f_ratio[0], f_ratio[1])
    print

  avg_f_ratio[0] /= len(queries)
  avg_f_ratio[1] /= len(queries)
  print "avg\t %.2f/%.2f\n" % (avg_f_ratio[0], avg_f_ratio[1])


if __name__ == "__main__":
  main()
