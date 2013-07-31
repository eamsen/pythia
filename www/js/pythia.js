// Copyright 2013 Eugen Sawin <esawin@me73.com>
var server = "http://" + window.location.hostname + ":" + window.location.port;
var broccoli_server = "http://broccoli.informatik.uni-freiburg.de";

String.prototype.ReplaceAll = function(find, replace) {
    return this.replace(new RegExp(find, 'g'), replace);
}

var options = {
  v: "0.1.3",
  show_performance: false,
  show_query_analysis: false,
  show_target_types: false,
  show_semantic_query: false,
  show_semantic_entity_chart: false,
  show_scoring: false,
  show_entity_chart: false,
  show_entity_table: false,
  show_evaluation: false,
  show_documents: false
};

var server_options = {
  // Freebase target type detection.
  fbtt: 1
};

function SearchResult(query) {
  this.query = query;
  this.keywords = [];
  this.target_keywords = [];
  this.entities = [];
  this.type_scores = {};
  this.coarse_type = "unknown";
  return this;
}

SearchResult.prototype.CoarseType = function() {
  if (this.coarse_type === "unknown") {
    var max = [0, "unknown"];
    for (var k in this.type_scores) {
      if (this.type_scores[k] > max[0]) {
        max[0] = this.type_scores[k];
        max[1] = k;
      }
    }
    this.coarse_type = max[1];
  }
  return this.coarse_type;
}

var search_result = new SearchResult(null);

var entities = [];
var query = null;
var scoring_options = {
  v: "0.1.5",
  cfw: [0.6, 0.58, 0.55, 0.53, 0.5, 0.48, 0.45, 0.43, 0.4, 0.38, 0.30],
  sfw: [0.6, 0.58, 0.55, 0.53, 0.5, 0.48, 0.45, 0.43, 0.4, 0.38, 0.30],
  cdfw: 0.25,
  sdfw: 0.25,
  ontology_filter: 1,
  similarity_filter: 1,
  coarse_type_filter: 1,
  yago_type_filter: 0,
  fb_type_filter: 0,
  entity_clustering: 0,
};

var ground_truth = {
  v: "0.0.2",
  valid: false,
  queries: [],
  entities: [],
  keywords: [],
  target_keywords: [],
  coarse_types: [],
};

var evaluation = {
  v: "0.0.6",
  valid: false,
  next: 0,
  f_s: [],
  approx_f_s: [],
  sem_f_s: [],
  sem_approx_f_s: [],
  recalls: [],
  sem_recalls: [],
  recalls_s: [],
  precisions_10: [],
  sem_precisions_10: [],
  precisions_r: [],
  sem_precisions_r: [],
  precisions_s: [],
  sem_precisions_s: [],
  approx_recalls: [],
  approx_recalls_s: [],
  approx_precisions_10: [],
  sem_approx_recalls: [],
  sem_approx_precisions_10: [],
  approx_precisions_r: [],
  sem_approx_precisions_r: [],
  approx_precisions_s: [],
  sem_approx_precisions_s: [],
  avg_f_s: 0,
  avg_approx_f_s: 0,
  avg_sem_f_s: 0,
  avg_sem_approx_f_s: 0,
  avg_recall: 0,
  avg_recall_s: 0,
  avg_precision_10: 0,
  avg_sem_precision_10: 0,
  avg_precision_r: 0,
  avg_sem_precision_r: 0,
  avg_precision_s: 0,
  avg_sem_precision_s: 0,
  avg_approx_recall: 0,
  avg_approx_recall_s: 0,
  avg_approx_precision_10: 0,
  avg_sem_approx_precision_10: 0,
  avg_approx_precision_r: 0,
  avg_sem_approx_precision_r: 0,
  avg_approx_precision_s: 0,
  avg_sem_approx_precision_s: 0,
};

var value_prec = 2;

function ServerOptions() {
  var o = "";
  for (var i in server_options) {
    if (o.length > 0) {
      o += "&";
    }
    o += i + "=" + server_options[i];
  }
  return o;
}

function UrlFormat(q) {
  return '"' + q + '"';
}

function UserFormat(q) {
  var qc = decodeURIComponent(q);
  return qc.substr(1, qc.length - 2);
}

function UserQuery(q) {
  if (q) {
    document.getElementById("query").value = q.toLowerCase(); 
  }
  return document.getElementById("query").value; 
}

function UrlQuery(q) {
  if (q) {
    window.location.search = '?q=' + q.toLowerCase();
  }
  return window.location.search.substr(3);
}

function TrimStr(s, length) {
  var max_length = length || 20;
  if (s.length <= max_length) {
    return s;
  }
  var cut  = s.substr(0, max_length);
  cut = cut.substr(0, Math.min(cut.length, cut.lastIndexOf(" ")));
  cut += " ...";
  return cut;
}

function InitVSlider(s, v, e) {
  $("#score-slider-" + s).slider({
    min: 0.0,
    max: 1.0,
    step: 0.01,
    orientation: "vertical",
    value: 1.0 - v,
    tooltip: "hide",
    handle: "square"
  }).on("slideStop", e);
}

function SetOptionFunc(opt, i) {
  return function (ev) {
    if (i !== undefined) {
      scoring_options[opt][i] = 1.0 - ev.value;
    } else {
      scoring_options[opt] = 1.0 - ev.value;
    }
    $.cookie("scoring_options", scoring_options);
    ScoreEntities(query, entities);
    SortEntities(entities);
    FilterEntities(entities);
    UpdateEntityTable(entities);
    UpdateEntityChart(entities);
    UpdateSemanticQuery(entities);
    EvaluateResults();
  };
}

function InitSliders() {
  for (var i = 0; i < 11; ++i) {
    InitVSlider("cfw" + i, scoring_options.cfw[i], SetOptionFunc("cfw", i));
    InitVSlider("sfw" + i, scoring_options.sfw[i], SetOptionFunc("sfw", i));
  }
  InitVSlider("cdfw", scoring_options.cdfw, SetOptionFunc("cdfw"));
  InitVSlider("sdfw", scoring_options.sdfw, SetOptionFunc("sdfw"));
}

function Search(query, opts) {
  if (opts == undefined) {
    opts = "";
  }
  opts += "&" + ServerOptions();
  $.ajax({url: server + "/",
    data: "qf=" + query + opts, 
    dataType: "json",
    success: SearchCallback});
}

function QueryTypeInfo(entities, eval) {
  if (eval === undefined) {
    eval = -1;
  }
  var k = 0;
  var es = "";
  for (var i = 0; i < entities.length; ++i) {
    if (entities[i][8]) {
      continue;
    }
    if (++k == 10) {
      break;
    }
    if (k > 1) {
      es += '+';
    }
    es += entities[i][0] + ':' + entities[i][5];
  }
  $.ajax({url: server + "/",
    data: "ti=" + es + "&" + ServerOptions() + "&eval=" + eval,
    dataType: "json",
    success: TypeInfoCallback});
}

function PrefixEditDistance(prefix, word) {
  var min_dist = Math.max(prefix.length, word.length);
  if (prefix.length > word.length) {
    return min_dist;
  }
  var dists = new Array(new Array(word.length + 1), new Array(word.length + 1));
  var di = 0;
  for (var i = 0; i < dists[di].length; ++i) {
    dists[di][i] = i;
  }
  for (var w2 = 0; w2 < prefix.length; ++w2) {
    dists[1 - di][0] = dists[di][0] + 1;
    for (var w1 = 0; w1 < word.length; ++w1) {
      dists[1 - di][w1 + 1] = Math.min(Math.min(dists[di][w1 + 1],
                                                dists[1 - di][w1]) + 1,
                                       dists[di][w1] + (word[w1] != prefix[w2]));
    }
    di = 1 - di;
  }
  dists[di].sort(function (a, b) { return a - b; });
  min_dist = dists[di][0];
  return min_dist;
}

function MergeEntities(e1, e2) {
  var e = [];
  var num_words1 = e1[0].split(" ").length;
  var num_words2 = e2[0].split(" ").length;
  if ((e1[1] == "person" && num_words1 > num_words2 && num_words1 < 5) ||
      (e1[1] != "person" && e1[2] > e2[2])) {
    e[0] = e1[0];
  } else {
    e[0] = e2[0];
  }
  if (e1[2] > e2[2] && e1[1] != "misc") {
    e[1] = e1[1];
  } else {
    e[1] = e2[1];
  }
  e[2] = e1[2] + e2[2];
  e[3] = e1[3] + e2[3];
  e[4] = e1[4] + e2[4];
  e[5] = e1[5] + e2[5];
  e[6] = e1[6].concat(e2[6]);
  e[7] = e1[7].concat(e2[7]);
  return e; 
}

function ClusterEntities(entities) {
  if (!scoring_options.entity_clustering) {
    return entities;
  }
  var new_entities = new Array(entities, []);
  var ei = 0;
  while (new_entities[0].length != new_entities[1].length) {
    new_entities[1 - ei] = [];
    var ignore = new Array(new_entities[ei].length);
    for (var i = 0; i < new_entities[ei].length; ++i) {
      if (ignore[i]) {
        continue;
      }
      var name = new_entities[ei][i][0].split(" ");
      for (var j = i + 1; j < new_entities[ei].length; ++j) {
        var other_name = new_entities[ei][j][0].split(" ");
        if (IsSimilar(name, other_name)) {
          new_entities[1 - ei].push(MergeEntities(new_entities[ei][i],
                                                  new_entities[ei][j]));
          ignore[j] = true;
          break;
        }
      }
      if (j == new_entities[ei].length) {
        new_entities[1 - ei].push(new_entities[ei][i]);
      }
    }
    ei = 1 - ei;
  }
  return new_entities[0];
}

function IsSimilar(words1, words2) {
  var num_similar = 0;
  var num_shorts = [words1.length, words2.length];
  for (var i = 0; i < words2.length; ++i) {
    if (words2[i].length > 3) {
      --num_shorts[1];
    }
  }
  for (var j = 0; j < words1.length; ++j) {
    if (words1[j].length > 3) {
      --num_shorts[0];
      continue;
    }
    for (var i = 0; i < words2.length; ++i) {
      var k = 0;
      while (i + k < words2.length &&
             words1[j][k] == words2[i + k][0]) {
        ++k;
      }
      if (k == words1[j].length) {
        ++num_similar;
        break;
      }
    }
  }
  for (var i = 0; i < words2.length; ++i) {
    if (words2[i].length == 0) {
      continue;
    }
    for (var j = 0; j < words1.length; ++j) {
      if (words1[j].length == 0) {
        continue;
      }
      var ped = 0;
      if (words2[i].length > words1[j].length) {
        ped = PrefixEditDistance(words1[j], words2[i]);
      } else {
        ped = PrefixEditDistance(words2[i], words1[j]);
      }
      if (ped <= Math.min(words2[i].length, words1[j].length) * 0.3) {
        ++num_similar;
      }
    }
  }
  return num_similar > (Math.min(words1.length, words2.length) -
                        Math.min(num_shorts[0], num_shorts[1])) * 0.6;
}

function PreFilter(query, entity) {
  if (scoring_options.ontology_filter &&
      entity[4] == 0) {
    return false;
  }
  if (scoring_options.similarity_filter) {
    var entity_words = entity[0].split(" ");
    var query_words = query.split(" ");
    if (IsSimilar(entity_words, query_words)) {
      return false;
    }
  }
  return true;
}

function PostFilter(entity) {
}

function Score(query, entity) {
  var score = {filtered: false, cf: 0.0, sf: 0.0, cdf: 0.0, sdf: 0.0};
  if (!PreFilter(query, entity)) {
    score.filtered = true;
    // return score;
  }
  var content_freq = entity[2];
  var snippet_freq = entity[3];
  var corpus_freq = entity[4];
  var content_freqs = entity[6];
  var snippet_freqs = entity[7];
  var content_doc_freq = content_freqs.length;
  var snippet_doc_freq = snippet_freqs.length;

  var cfw = scoring_options.cfw;
  var sfw = scoring_options.sfw;
  var cdfw = scoring_options.cdfw;
  var sdfw = scoring_options.sdfw;

  for (var i = 0; i < content_freqs.length; ++i) {
    score.cf += cfw[content_freqs[i][0]] * content_freqs[i][1];
  }
  for (var i = 0; i < snippet_freqs.length; ++i) {
    score.sf += sfw[snippet_freqs[i][0]] * snippet_freqs[i][1];
  }
  var corpus_freq_div = Math.log(corpus_freq + 400);
  score.cf /= corpus_freq_div;
  score.sf /= corpus_freq_div;
  score.cdf = content_doc_freq * content_doc_freq;
  score.sdf = snippet_doc_freq * snippet_doc_freq;
  return score;
}

function UpdateSemanticQuery(entities) {
  QueryTypeInfo(entities);
  var coarse_target_types = search_result.CoarseType();
  $("#coarse-target-types").html(coarse_target_types.toUpperCase());
}

function UpdateEntityTable(entities) {
  var ex_score = [Number.MAX_VALUE, Number.MIN_VALUE];
  var ex_content_freq = [Number.MAX_VALUE, Number.MIN_VALUE];
  var ex_corpus_freq = [Number.MAX_VALUE, Number.MIN_VALUE];
  var entity_table = "<thead><tr><th>Entity</th><th>Coarse Type</th>" +
    "<th>Content Freq</th>" +
    "<th>Snippet Freq</th>" +
    "<th>Document Freq</th>" +
    "<th>Corpus Freq</th>" +
    "<th>Score</th>" +
    "</tr></thead><tbody>";
  for (var i in entities) {
    var name = entities[i][0];
    var type = entities[i][1];
    var content_freq = entities[i][2];
    var snippet_freq = entities[i][3];
    var corpus_freq = entities[i][4];
    var score = entities[i][5];
    var doc_freq = entities[i][6].length;
    var filtered = entities[i][8];
    if (!filtered) {
      entity_table += "<tr>";
    } else {
      entity_table += "<tr class=\"error\">";
    }
    entity_table += "<td>" + name.toUpperCase() + "</td>" +
      "<td>" + type + "</td>" +
      "<td>" + content_freq + "</td>" +
      "<td>" + snippet_freq + "</td>" +
      "<td>" + doc_freq + "</td>" +
      "<td>" + corpus_freq + "</td>" +
      "<td>" + score.toFixed(value_prec) + "</td></tr>";
  }
  entity_table += "</tbody>";
  $("#entity-table").html(entity_table);
  ApplySortability();
}

function TypeInfoCallback(data, status, xhr) {
  var type_scores = {};
  var best_type = [Number.MIN_VALUE, "unknown"];
  var k = data.yago_types.length;
  for (var i = 0; i < data.yago_types.length; ++i) {
    var entity_name = data.yago_types[i][0];
    var entity_score = data.yago_types[i][1];
    var yago_types = data.yago_types[i][2];
    for (var j = 0; j < yago_types.length; ++j) {
      if (// yago_types[j][1] > 2400000 ||
          yago_types[j][1] < 50 ||
          yago_types[j][0].length > 18) {
        continue;
      }
      if (type_scores[yago_types[j][0]] === undefined) {
        type_scores[yago_types[j][0]] = [0, yago_types[j][1]];
      }
      var itf = 17 - Math.log(yago_types[j][1] + 100000);
      type_scores[yago_types[j][0]][0] += entity_score * itf;
      var score = type_scores[yago_types[j][0]][0];
      if (score > best_type[0]) {
        best_type[0] = score;
        best_type[1] = yago_types[j][0];
      }
    }
  }
  // var sorted = Object.keys(type_scores).sort(function(a, b) {
  //   return type_scores[b][0] - type_scores[a][0];
  // });
  // console.log(sorted);
  var type = best_type[1].substr(0, 1).toUpperCase() +
      best_type[1].substr(1, best_type[1].length - 1);
  if (data.eval == -1) {
    $("#yago-target-types").html(best_type[1].toUpperCase());
    var broccoli_query = "$1 is-a " + type +
        ";$1 occurs-with " + search_result.keywords.join(" ");
    if (search_result.keywords.length == 0) {
      broccoli_query += search_result.target_keywords.join(" ");
    }
    $("#broccoli-query-area").html(broccoli_query);
  } else {
    var broccoli_query = "$1 is-a " + type +
        ";$1 occurs-with " + ground_truth.keywords[data.eval].join(" ");
    if (ground_truth.keywords[data.eval].length == 0) {
      broccoli_query += ground_truth.target_keywords[data.eval].join(" ");
    }
  }
  console.log(broccoli_query);
  BroccoliSearch(broccoli_query, data.eval);
}

function SearchCallback(data, status, xhr) {
  if (data.entity_extraction === undefined) {
    return;
  }
  data.entity_extraction.entity_items =
      ClusterEntities(data.entity_extraction.entity_items);
  if (data.eval !== undefined) {
    UpdateEvaluation(data);
    return;
  }
  var durations = [["Procedure", "Duration [ms]"],
      // ["Total", data.duration / 1000],
      ["Query Analysis", data.query_analysis.duration / 1000],
      ["Document Retrieval", data.document_retrieval.duration / 1000],
      ["Entity Extraction", data.entity_extraction.duration / 1000]];
  UpdatePerformanceChart(durations, data.duration / 1000);
  var target_keywords = {};
  query = data.query_analysis.keywords.slice(0).join(" ");
  for (var i in data.query_analysis.target_keywords) { 
    var keywords = data.query_analysis.target_keywords[i].split(" ");
    query += " " + data.query_analysis.target_keywords[i];
    for (var j in keywords) {
      target_keywords[keywords[j]] = true;
    }
  }
  var keywords = {};
  for (var i in data.query_analysis.keywords) { 
    var keyword = data.query_analysis.keywords[i];
    keywords[keyword] = true;
    keywords[keyword + "'s"] = true;
  }
  var query_analysis = "";
  for (var i in data.query_analysis.query) {
    var word = data.query_analysis.query[i];
    if (word in target_keywords) {
      query_analysis += "<a href=\"#\" data-placement=\"top\" " +
        "data-toggle=\"tooltip\" title=\"target keyword\">" +
        "<span class=\"label label-info\">" + word.toUpperCase() + "</span></a>";
    } else if (word in keywords) {
      query_analysis += "<a href=\"#\" data-placement=\"top\" " +
        "data-toggle=\"tooltip\" title=\"keyword\">" +
        "<span class=\"label label-success\">" + word.toUpperCase()+ "</span></a>";
    } else {
      query_analysis += "<a href=\"#\" data-placement=\"top\" " +
        "data-toggle=\"tooltip\" title=\"no keyword\">" +
        "<span class=\"label\">" + word.toUpperCase()+ "</span></a>";
    }
    query_analysis += " ";
  } 
  $("#query-analysis-area").html(query_analysis);

  var documents = "";
  for (var i in data.document_retrieval.documents) {
    var title = data.document_retrieval.documents[i]["htmlTitle"];
    var snippet = data.document_retrieval.documents[i]["snippet"];
    var link = data.document_retrieval.documents[i]["link"];
    var link_name = data.document_retrieval.documents[i]["displayLink"]
    var element = "<p>" + "<a href=\"" + link + "\"><h8>" + title  + "</h8></a>"
        + "<br><span class=\"document-snippet\">" + snippet + "</span><br>"
        + "<a href=\"" + link + "\">" + link_name + "</a></p>";
    documents += element;
  }
  $("#result-area").html(documents);

  search_result.keywords = data.query_analysis.keywords;
  search_result.target_keywords = data.query_analysis.target_keywords;
  search_result.target_keywords = data.query_analysis.target_keywords;
  search_result.entities = data.entity_extraction.entity_items;
  // console.log(search_result)

  entities = data.entity_extraction.entity_items;
  ScoreEntities(query, entities);
  SortEntities(entities);
  FilterEntities(entities);
  UpdateEntityTable(entities);
  UpdateEntityChart(entities);
  UpdateSemanticQuery(entities);

  var yago_target_types = "";
  $("#yago-target-types").html(yago_target_types);
  var fb_target_types = "";
  $("#fb-target-types").html(fb_target_types);
}

function BroccoliSearch(query, eval) {
  if (eval === undefined) {
    eval = -1;
  }
  $.ajax({url: broccoli_server + "/api/?s=" + query +
    "&query=$1&hofhitgroups=8&nofclasses=0&nofinstances=9999&nofrelations=0&nofwords=0" +
    "&format=jsonp&callback=BroccoliSearchCallback&callback-argument=" + eval,
    dataType: "jsonp",
    jsonp: false
  });
}

function BroccoliSearchCallback(data, arg) {
  arg = parseInt(arg);

  if (data.result.status !== "OK") {
    console.log(data);
    return;
  }

  var entities = data.result.res.instances.data;
  if (arg == -1) {
    UpdateSemanticEntityChart(entities);
  } else {
    UpdateSemanticEvaluation(entities, arg);
  }
}

function UpdatePerformanceChart(durations, total) {
  var data = google.visualization.arrayToDataTable(durations);
  var options = {
    backgroundColor: {fill: "transparent", stroke: "#f4f8f7", strokeWidth: 4},
    colors: ["#f4f8f7", "#d0d6aa", "#c93a3e", "#51bab6"],
    fontName: "Lato",
    chartArea: {left:40, top:20, height:"86%", width:"82%"},
    legend: {textStyle: {color: "#f4f8f7"}},
    vAxis: {textStyle: {color: "#f4f8f7"}},
    hAxis: {textStyle: {color: "#f4f8f7"}}
  };
  var chart = new google.visualization.PieChart(
      document.getElementById("performance-chart2"));
  chart.draw(data, options);
  durations.push(["Total", total]); 
  data = google.visualization.arrayToDataTable(durations);
  chart = new google.visualization.BarChart(
      document.getElementById("performance-chart1"));
  options = {
    backgroundColor: {fill: "transparent", stroke: "#f4f8f7", strokeWidth: 4},
    colors: ["#f4f8f7", "#d0d6aa", "#c93a3e", "#51bab6"],
    fontName: "Lato",
    chartArea: {left:120, top:20, height:"78%", width:"62%"},
    legend: {textStyle: {color: "#f4f8f7"}},
    vAxis: {textStyle: {color: "#f4f8f7"}},
    hAxis: {textStyle: {color: "#f4f8f7"}}
  };
  chart.draw(data, options);
}

function EvaluateResults(init) {
  if (init === undefined || init == true) {
    evaluation.valid = false;
    evaluation.next = 0;
  }
  if (!ground_truth.valid) {
    GroundTruthRequest();
  } else if (evaluation.valid) {
    RenderEvaluation(evaluation);
    return;
  } else {
    if (evaluation.next >= ground_truth.queries.length) {
      // evaluation.valid = true;
      // evaluation.next = 0;
      // $.cookie("evaluation", evaluation);
      // sessionStorage.setItem("evaluation", JSON.stringify(evaluation));
      // ApplySortability();
    } else {
      Search(ground_truth.queries[evaluation.next], "&eval=" + evaluation.next);
      ++evaluation.next;
    }
  }
}

function RenderEvaluation(evaluation) {
  var table = "<thead><tr>" +
    "<th>Id</th>" +
    "<th>Query</th>" +
    "<th>Recall</th>" +
    "<th>Recall@S</th>" +
    "<th>P@10</th>" +
    "<th>P@R</th>" +
    "<th>P@S</th>" +
    "<th>F@S</th>" +
    "</tr></thead><tbody>" +
    "<tr class='error'><td>0.1</td>" + 
    "<td>FULL-TEXT AVERAGE</td>" +
    "<td id='evaluation-1-recall-0'>" +
    evaluation.avg_recall.toFixed(value_prec) +
    " <span>[" + evaluation.avg_approx_recall.toFixed(value_prec) +
    "]</span></td>" +
    "<td id='evaluation-1-recalls-0'>" +
    evaluation.avg_recall_s.toFixed(value_prec) +
    " <span>[" + evaluation.avg_approx_recall_s.toFixed(value_prec) +
    "]</span></td>" +
    "<td id='evaluation-1-prec10-0'>" +
    evaluation.avg_precision_10.toFixed(value_prec) +
    " <span>[" + evaluation.avg_approx_precision_10.toFixed(value_prec) +
    "]</span></td>" +
    "<td id='evaluation-1-precr-0'>" +
    evaluation.avg_precision_r.toFixed(value_prec) +
    " <span>[" + evaluation.avg_approx_precision_r.toFixed(value_prec) +
    "]</span></td>" +
    "<td id='evaluation-1-precs-0'>" +
    evaluation.avg_precision_s.toFixed(value_prec) +
    " <span>[" + evaluation.avg_approx_precision_s.toFixed(value_prec) +
    "]</span></td>" +
    "<td id='evaluation-1-fs-0'>" +
    evaluation.avg_f_s.toFixed(value_prec) +
    " <span>[" + evaluation.avg_approx_f_s.toFixed(value_prec) +
    "]</span></td>" +
    "</tr>" +
    "<tr class='error2'><td>0.2</td>" + 
    "<td>SEMANTIC AVERAGE</td>" +
    "<td id='evaluation-2-recall-0'>" +
    evaluation.avg_sem_recall.toFixed(value_prec) +
    " <span>[" + evaluation.avg_sem_approx_recall.toFixed(value_prec) +
    "]</span></td>" +
    "<td id='evaluation-2-recalls-0'>" +
    evaluation.avg_sem_recall.toFixed(value_prec) +
    " <span>[" + evaluation.avg_sem_approx_recall.toFixed(value_prec) +
    "]</span></td>" +
    "<td id='evaluation-2-prec10-0'>" +
    evaluation.avg_sem_precision_10.toFixed(value_prec) +
    " <span>[" + evaluation.avg_sem_approx_precision_10.toFixed(value_prec) +
    "]</span></td>" +
    "<td id='evaluation-2-precr-0'>" +
    evaluation.avg_sem_precision_r.toFixed(value_prec) +
    " <span>[" + evaluation.avg_sem_approx_precision_r.toFixed(value_prec) +
    "]</span></td>" +
    "<td id='evaluation-2-precs-0'>" +
    evaluation.avg_sem_precision_s.toFixed(value_prec) +
    " <span>[" + evaluation.avg_sem_approx_precision_s.toFixed(value_prec) +
    "]</span></td>" +
    "<td id='evaluation-2-fs-0'>" +
    evaluation.avg_sem_f_s.toFixed(value_prec) +
    " <span>[" + evaluation.avg_sem_approx_f_s.toFixed(value_prec) +
    "]</span></td>" +
    "</tr>";

  for (var i = 0; i < ground_truth.queries.length; ++i) {
    var query = ground_truth.queries[i] || 0.0;
    var recall = evaluation.recalls[i] || 0.0;
    var recall_s = evaluation.recalls_s[i] || 0.0;
    var precision_10 = evaluation.precisions_10[i] || 0.0;
    var precision_r = evaluation.precisions_r[i] || 0.0;
    var precision_s = evaluation.precisions_s[i] || 0.0;
    var f_s = evaluation.f_s[i] || 0.0;
    var approx_recall = evaluation.approx_recalls[i] || 0.0;
    var approx_recall_s = evaluation.approx_recalls_s[i] || 0.0;
    var approx_precision_10 = evaluation.approx_precisions_10[i] || 0.0;
    var approx_precision_r = evaluation.approx_precisions_r[i] || 0.0;
    var approx_precision_s = evaluation.approx_precisions_s[i] || 0.0;
    var approx_f_s = evaluation.approx_f_s[i] || 0.0;

    var sem_recall = evaluation.sem_recalls[i] || 0.0;
    var sem_precision_10 = evaluation.sem_precisions_10[i] || 0.0;
    var sem_precision_r = evaluation.sem_precisions_r[i] || 0.0;
    var sem_precision_s = evaluation.sem_precisions_s[i] || 0.0;
    var sem_f_s = evaluation.sem_f_s[i] || 0.0;
    var sem_approx_recall = evaluation.sem_approx_recalls[i] || 0.0;
    var sem_approx_precision_10 = evaluation.sem_approx_precisions_10[i] || 0.0;
    var sem_approx_precision_r = evaluation.sem_approx_precisions_r[i] || 0.0;
    var sem_approx_precision_s = evaluation.sem_approx_precisions_s[i] || 0.0;
    var sem_approx_f_s = evaluation.sem_approx_f_s[i] || 0.0;

    table += "<tr><td>" + (i + 1) + ".1</td>" + 
      "<td><a href='" + server + "/?q=\"" + query.ReplaceAll("'", "&#39;") +
      "\"'>" + TrimStr(query)  + "</a></td>" +
      "<td>" + recall.toFixed(value_prec) +
      "<span class='red'> [" + approx_recall.toFixed(value_prec) +
      "]</span></td>" +
      "<td>" + recall_s.toFixed(value_prec) +
      "<span class='red'> [" + approx_recall_s.toFixed(value_prec) +
      "]</span></td>" +
      "<td>" + precision_10.toFixed(value_prec) +
      "<span class='red'> [" + approx_precision_10.toFixed(value_prec) +
      "]</span></td>" +
      "<td>" + precision_r.toFixed(value_prec) +
      "<span class='red'> [" + approx_precision_r.toFixed(value_prec) +
      "]</span></td>" +
      "<td>" + precision_s.toFixed(value_prec) +
      "<span class='red'> [" + approx_precision_s.toFixed(value_prec) +
      "]</span></td>" +
      "<td>" + f_s.toFixed(value_prec) +
      "<span class='red'> [" + approx_f_s.toFixed(value_prec) +
      "]</span></td>" +
      "</tr>" +
      "<tr><td>" + (i + 1) + ".2</td>" + 
      "<td></td>" +
      "<td>" + sem_recall.toFixed(value_prec) +
      "<span class='red'> [" + sem_approx_recall.toFixed(value_prec) +
      "]</span></td>" +
      "<td>" + sem_recall.toFixed(value_prec) +
      "<span class='red'> [" + sem_approx_recall.toFixed(value_prec) +
      "]</span></td>" +
      "<td>" + sem_precision_10.toFixed(value_prec) +
      "<span class='red'> [" + sem_approx_precision_10.toFixed(value_prec) +
      "]</span></td>" +
      "<td>" + sem_precision_r.toFixed(value_prec) +
      "<span class='red'> [" + sem_approx_precision_r.toFixed(value_prec) +
      "]</span></td>" +
      "<td>" + sem_precision_s.toFixed(value_prec) +
      "<span class='red'> [" + sem_approx_precision_s.toFixed(value_prec) +
      "]</span></td>" +
      "<td>" + sem_f_s.toFixed(value_prec) +
      "<span class='red'> [" + sem_approx_f_s.toFixed(value_prec) +
      "]</span></td>" +
      "</tr>";
  }
  table += "</tbody>";
  $("#evaluation-table").html(table);
}

function FMeasure(recall, precision) {
  var s = recall + precision;
  if (s > 0) {
    return 2.0 * recall * precision / s;
  }
  return 0;
}

function MeasureStats(entities, eval) {
  var relevant = {};
  var num_rel = ground_truth.entities[eval].length;

  var ret = {
    recall: 0,
    recall_s: 0,
    precision_10: 0,
    precision_r: 0,
    precision_s: 0,
    approx_recall: 0,
    approx_recall_s: 0,
    approx_precision_10: 0,
    approx_precision_r: 0,
    approx_precision_s: 0,
    f_s: 0,
    approx_f_s: 0,
  };

  // Set all relevant entities result ranks to 0, indicating not found.
  for (var i = 0; i < num_rel; ++i) {
    relevant[ground_truth.entities[eval][i]] = 0;
  }

  // Compute maxima and average scores, this is required for cutoff testing.
  // There is no cutoff for semantic entities, so don't compute anything if
  // there are no scores.
  var sem_eval = entities.length === 0 || entities[0].length === 2;
  if (!sem_eval) {
    var scores = [];
    var ex_score = [Number.MAX_VALUE, Number.MIN_VALUE];
    for (var i = 0; i < entities.length; ++i) {
      var name = entities[i][0];
      var score = entities[i][5];
      var filtered = entities[i][8];
      if (!filtered && relevant[name] === 0) {
        if (score < ex_score[0]) {
          ex_score[0] = score;
        }
        if (score > ex_score[1]) {
          ex_score[1] = score;
        }
        scores.push(score);
      }
    }
    var avg_score = ExpMovAvg(scores, 10);
  }

  // Compute recall and precisions.
  var num_selected = 0;
  var rank = 1;
  for (var i = 0; i < entities.length; ++i) {
    var name = entities[i][0];
    var score = entities[i][5];
    var filtered = entities[i][8];
    if (!filtered && relevant[name] === 0) {
      relevant[name] = rank;
      ++ret.recall;
      if (rank <= 10) {
        ++ret.precision_10;
      }
      if (rank <= num_rel) {
        ++ret.precision_r;
      }
      if (sem_eval || !ScoreCutoff(score, avg_score, ex_score[0], ex_score[1])) {
        ++ret.precision_s;
      }
    }
    num_selected += !filtered && (sem_eval ||
                    !ScoreCutoff(score, avg_score, ex_score[0], ex_score[1]));
    rank += !filtered;
  }

  // Compute approximate recall and precisions.
  var rank = 1;
  for (var i = 0; i < entities.length; ++i) {
    var name = entities[i][0];
    var name_array = name.split(" ");
    var score = entities[i][5];
    var filtered = entities[i][8];
    if (!filtered && (relevant[name] === undefined || relevant[name] > rank)) {
      for (var j = 0; j < num_rel; ++j) {
        var truth_name = ground_truth.entities[eval][j];
        if ((relevant[truth_name] === 0 || relevant[truth_name] > rank) &&
            IsSimilar(name_array, truth_name.split(" "))) {
          var prev_match = relevant[truth_name];
          ret.approx_recall += prev_match === 0;
          if (rank <= 10) {
            ret.approx_precision_10 += prev_match > 10 || prev_match === 0;
          }
          if (rank <= num_rel) {
            ret.approx_precision_r += prev_match > num_rel || prev_match === 0;
          }
          if (sem_eval ||
              !ScoreCutoff(score, avg_score, ex_score[0], ex_score[1])) {
            ret.approx_precision_s += prev_match > num_selected ||
                                      prev_match === 0;
          }
          relevant[truth_name] = rank;
          break;
        }
      }
    } 
    rank += !filtered;
  }

  // Avoid division by zero.
  if (num_selected === 0) {
    num_selected = 1;
  }

  ret.approx_recall = (ret.recall + ret.approx_recall) / num_rel;
  ret.approx_recall_s = (ret.precision_s + ret.approx_precision_s) / num_rel;
  ret.approx_precision_10 = (ret.precision_10 + ret.approx_precision_10) / 10;
  ret.approx_precision_r = (ret.precision_r + ret.approx_precision_r) / num_rel;
  ret.approx_precision_s = (ret.precision_s + ret.approx_precision_s) /
                           num_selected;

  ret.recall /= num_rel;
  ret.recall_s = ret.precision_s / num_rel;
  ret.precision_10 /= 10;
  ret.precision_r /= num_rel;
  ret.precision_s /= num_selected;

  ret.f_s = FMeasure(ret.recall_s, ret.precision_s);
  ret.approx_f_s = FMeasure(ret.approx_recall_s, ret.approx_precision_s);

  return ret;
}

function Sum(a, b) {
  return a + b;
}

function UpdateSemanticEvaluation(entities_, eval) {
  var query = ground_truth.queries[eval];
  var num_data = ground_truth.queries.length;
  var entities = [];
  for (var i = 0; i < entities_.length; ++i) {
    var name = entities_[i].inst;
    var beg = name.lastIndexOf(":") + 1;
    var end = name.indexOf("(") - 1;
    if (end == -2) {
      end = name.length;
    }
    name = name.substr(beg, end - beg).ReplaceAll("_", " ").toLowerCase();
    var score = parseInt(entities_[i].score);
    entities.push([name, score]);
  }
  SortEntities(entities, 1);

  var m = MeasureStats(entities, eval);

  evaluation.sem_recalls[eval] = m.recall;
  evaluation.sem_f_s[eval] = m.f_s;
  evaluation.sem_precisions_10[eval] = m.precision_10;
  evaluation.sem_precisions_r[eval] = m.precision_r;
  evaluation.sem_precisions_s[eval] = m.precision_s;

  evaluation.sem_approx_recalls[eval] = m.approx_recall;
  evaluation.sem_approx_f_s[eval] = m.approx_f_s;
  evaluation.sem_approx_precisions_10[eval] = m.approx_precision_10;
  evaluation.sem_approx_precisions_r[eval] = m.approx_precision_r;
  evaluation.sem_approx_precisions_s[eval] = m.approx_precision_s;

  evaluation.avg_sem_recall = evaluation.sem_recalls.reduce(Sum) / num_data;
  evaluation.avg_sem_precision_10 =
      evaluation.sem_precisions_10.reduce(Sum) / num_data;
  evaluation.avg_sem_precision_r =
      evaluation.sem_precisions_r.reduce(Sum) / num_data;
  evaluation.avg_sem_precision_s =
      evaluation.sem_precisions_s.reduce(Sum) / num_data;
  evaluation.avg_sem_f_s =
      FMeasure(evaluation.avg_sem_recall, evaluation.avg_sem_precision_s);

  evaluation.avg_sem_approx_recall =
      evaluation.sem_approx_recalls.reduce(Sum) / num_data;
  evaluation.avg_sem_approx_precision_10 =
      evaluation.sem_approx_precisions_10.reduce(Sum) / num_data;
  evaluation.avg_sem_approx_precision_r =
      evaluation.sem_approx_precisions_r.reduce(Sum) / num_data;
  evaluation.avg_sem_approx_precision_s =
      evaluation.sem_approx_precisions_s.reduce(Sum) / num_data;
  evaluation.avg_sem_approx_f_s =
      FMeasure(evaluation.avg_sem_approx_recall,
               evaluation.avg_sem_approx_precision_s);

  $("#evaluation-2-recall-0").html(
      evaluation.avg_sem_recall.toFixed(value_prec) +
      " [" + evaluation.avg_sem_approx_recall.toFixed(value_prec) + "]");
  $("#evaluation-2-recalls-0").html(
      evaluation.avg_sem_recall.toFixed(value_prec) +
      " [" + evaluation.avg_sem_approx_recall.toFixed(value_prec) + "]");
  $("#evaluation-2-prec10-0").html(
      evaluation.avg_sem_precision_10.toFixed(value_prec) +
      " [" + evaluation.avg_sem_approx_precision_10.toFixed(value_prec) + "]");
  $("#evaluation-2-precr-0").html(
      evaluation.avg_sem_precision_r.toFixed(value_prec) +
      " [" + evaluation.avg_sem_approx_precision_r.toFixed(value_prec) + "]");
  $("#evaluation-2-precs-0").html(
      evaluation.avg_sem_precision_s.toFixed(value_prec) +
      " [" + evaluation.avg_sem_approx_precision_s.toFixed(value_prec) + "]");
  $("#evaluation-2-fs-0").html(
      evaluation.avg_sem_f_s.toFixed(value_prec) +
      " [" + evaluation.avg_sem_approx_f_s.toFixed(value_prec) + "]");

  $("#evaluation-2-recall-" + (eval + 1)).html(
      m.recall.toFixed(value_prec) +
      " <span class='red'>[" + m.approx_recall.toFixed(value_prec) +
      "]</span>");
  $("#evaluation-2-recalls-" + (eval + 1)).html(
      m.recall.toFixed(value_prec) +
      " <span class='red'>[" + m.approx_recall.toFixed(value_prec) +
      "]</span>");
  $("#evaluation-2-prec10-" + (eval + 1)).html(
      m.precision_10.toFixed(value_prec) +
      " <span class='red'>[" + m.approx_precision_10.toFixed(value_prec) +
      "]</span>");
  $("#evaluation-2-precr-" + (eval + 1)).html(
      m.precision_r.toFixed(value_prec) +
      " <span class='red'>[" + m.approx_precision_r.toFixed(value_prec) +
      "]</span>");
  $("#evaluation-2-precs-" + (eval + 1)).html(
      m.precision_s.toFixed(value_prec) +
      " <span class='red'>[" + m.approx_precision_s.toFixed(value_prec) +
      "]</span>");
  $("#evaluation-2-fs-" + (eval + 1)).html(
      m.f_s.toFixed(value_prec) +
      " <span class='red'>[" + m.approx_f_s.toFixed(value_prec) +
      "]</span>");

  if (eval == ground_truth.queries.length - 1) {
    evaluation.valid = true;
    evaluation.next = 0;
    SaveSessionItem("evaluation");
  }
}

function UpdateEvaluation(data) {
  var entities = data.entity_extraction.entity_items;
  var keywords = data.query_analysis.keywords;
  var query = data.query_analysis.keywords.slice(0).join(" ");
  var num_data = ground_truth.queries.length;
  for (var i = 0; i < data.query_analysis.target_keywords.length; ++i) { 
    query += " " + data.query_analysis.target_keywords[i];
  }
  ScoreEntities(query, entities);
  SortEntities(entities);
  FilterEntities(entities);

  var m = MeasureStats(entities, data.eval);

  evaluation.recalls[data.eval] = m.recall;
  evaluation.recalls_s[data.eval] = m.recall_s;
  evaluation.f_s[data.eval] = m.f_s;
  evaluation.precisions_10[data.eval] = m.precision_10;
  evaluation.precisions_r[data.eval] = m.precision_r;
  evaluation.precisions_s[data.eval] = m.precision_s;

  evaluation.approx_recalls[data.eval] = m.approx_recall;
  evaluation.approx_recalls_s[data.eval] = m.approx_recall_s;
  evaluation.approx_f_s[data.eval] = m.approx_f_s;
  evaluation.approx_precisions_10[data.eval] = m.approx_precision_10;
  evaluation.approx_precisions_r[data.eval] = m.approx_precision_r;
  evaluation.approx_precisions_s[data.eval] = m.approx_precision_s;

  if (data.eval == 0) {
    var table_init = "<thead><tr>" +
      "<th>Id</th>" +
      "<th>Query</th>" +
      "<th>Recall</th>" +
      "<th>Recall@S</th>" +
      "<th>P@10</th>" +
      "<th>P@R</th>" +
      "<th>P@S</th>" +
      "<th>F@S</th>" +
      "</tr></thead><tbody>" +
      "<tr class='error'><td>0.1</td>" + 
      "<td>FULL-TEXT AVERAGE</td>" +
      "<td id='evaluation-1-recall-0'>0.00" +
      " <span>[0.00]</span></td>" +
      "<td id='evaluation-1-recalls-0'>0.00" +
      " <span>[0.00]</span></td>" +
      "<td id='evaluation-1-prec10-0'>0.00" +
      " <span>[0.00]</span></td>" +
      "<td id='evaluation-1-precr-0'>0.00" +
      " <span>[0.00]</span></td>" +
      "<td id='evaluation-1-precs-0'>0.00" +
      " <span>[0.00]</span></td>" +
      "<td id='evaluation-1-fs-0'>0.00" +
      " <span>[0.00]</span></td>" +
      "</tr>" +
      "<tr class='error2'><td>0.2</td>" + 
      "<td>SEMANTIC AVERAGE</td>" +
      "<td id='evaluation-2-recall-0'>0.00" +
      " <span>[0.00]</span></td>" +
      "<td id='evaluation-2-recalls-0'>0.00" +
      " <span>[0.00]</span></td>" +
      "<td id='evaluation-2-prec10-0'>0.00" +
      " <span>[0.00]</span></td>" +
      "<td id='evaluation-2-precr-0'>0.00" +
      " <span>[0.00]</span></td>" +
      "<td id='evaluation-2-precs-0'>0.00" +
      " <span>[0.00]</span></td>" +
      "<td id='evaluation-2-fs-0'>0.00" +
      " <span>[0.00]</span></td>" +
      "</tr></tbody>";
    $("#evaluation-table").html(table_init);
  } else {
    evaluation.avg_recall = evaluation.recalls.reduce(Sum) / num_data;
    evaluation.avg_recall_s = evaluation.recalls_s.reduce(Sum) / num_data;
    evaluation.avg_precision_10 = evaluation.precisions_10.reduce(Sum) / num_data;
    evaluation.avg_precision_r = evaluation.precisions_r.reduce(Sum) / num_data;
    evaluation.avg_precision_s = evaluation.precisions_s.reduce(Sum) / num_data;
    evaluation.avg_f_s = FMeasure(evaluation.avg_recall_s, evaluation.avg_precision_s);

    evaluation.avg_approx_recall = evaluation.approx_recalls.reduce(Sum) / num_data;
    evaluation.avg_approx_recall_s = evaluation.approx_recalls_s.reduce(Sum) / num_data;
    evaluation.avg_approx_precision_10 = evaluation.approx_precisions_10.reduce(Sum) / num_data;
    evaluation.avg_approx_precision_r = evaluation.approx_precisions_r.reduce(Sum) / num_data;
    evaluation.avg_approx_precision_s = evaluation.approx_precisions_s.reduce(Sum) / num_data;
    evaluation.avg_approx_f_s = FMeasure(evaluation.avg_approx_recall_s, evaluation.avg_approx_precision_s);
  }
  if (data.eval < num_data) {
    var query = ground_truth.queries[data.eval];
    var row = "<tr><td>" + (data.eval + 1) + ".1</td>" + 
      "<td><a href='" + server + "/?q=\"" + query.ReplaceAll("'", "&#39;") +
      "\"'>" + TrimStr(query)  + "</a></td>" +
      "<td>" + m.recall.toFixed(value_prec) +
      "<span class='red'> [" + m.approx_recall.toFixed(value_prec) + "]</span></td>" +
      "<td>" + m.recall_s.toFixed(value_prec) +
      "<span class='red'> [" + m.approx_recall_s.toFixed(value_prec) + "]</span></td>" +
      "<td>" + m.precision_10.toFixed(value_prec) +
      "<span class='red'> [" + m.approx_precision_10.toFixed(value_prec) + "]</span></td>" +
      "<td>" + m.precision_r.toFixed(value_prec) +
      "<span class='red'> [" + m.approx_precision_r.toFixed(value_prec) + "]</span></td>" +
      "<td>" + m.precision_s.toFixed(value_prec) +
      "<span class='red'> [" + m.approx_precision_s.toFixed(value_prec) + "]</span></td>" +
      "<td>" + m.f_s.toFixed(value_prec) +
      "<span class='red'> [" + m.approx_f_s.toFixed(value_prec) + "]</span></td>" +
      "</tr>" +
      "<tr><td>" + (data.eval + 1) + ".2</td>" + 
      "<td></td>" +
      "<td id='evaluation-2-recall-" + (data.eval + 1) + "'>0.00" + 
      " <span class='red'>[0.00]</span></td>" +
      "<td id='evaluation-2-recalls-" + (data.eval + 1) + "'>0.00" + 
      " <span class='red'>[0.00]</span></td>" +
      "<td id='evaluation-2-prec10-" + (data.eval + 1) + "'>0.00" + 
      " <span class='red'>[0.00]</span></td>" +
      "<td id='evaluation-2-precr-" + (data.eval + 1) + "'>0.00" + 
      " <span class='red'>[0.00]</span></td>" +
      "<td id='evaluation-2-precs-" + (data.eval + 1) + "'>0.00" + 
      " <span class='red'>[0.00]</span></td>" +
      "<td id='evaluation-2-fs-" + (data.eval + 1) + "'>0.00" + 
      " <span class='red'>[0.00]</span></td>" +
      "</tr>";
    $("#evaluation-table > tbody:last").append(row);
  }
  $("#evaluation-1-recall-0").html(
      evaluation.avg_recall.toFixed(value_prec) +
      " [" + evaluation.avg_approx_recall.toFixed(value_prec) + "]");
  $("#evaluation-1-recalls-0").html(
      evaluation.avg_recall_s.toFixed(value_prec) +
      " [" + evaluation.avg_approx_recall_s.toFixed(value_prec) + "]");
  $("#evaluation-1-prec10-0").html(
      evaluation.avg_precision_10.toFixed(value_prec) +
      " [" + evaluation.avg_approx_precision_10.toFixed(value_prec) + "]");
  $("#evaluation-1-precr-0").html(
      evaluation.avg_precision_r.toFixed(value_prec) +
      " [" + evaluation.avg_approx_precision_r.toFixed(value_prec) + "]");
  $("#evaluation-1-precs-0").html(
      evaluation.avg_precision_s.toFixed(value_prec) +
      " [" + evaluation.avg_approx_precision_s.toFixed(value_prec) + "]");
  $("#evaluation-1-fs-0").html(
      evaluation.avg_f_s.toFixed(value_prec) +
      " [" + evaluation.avg_approx_f_s.toFixed(value_prec) + "]");

  QueryTypeInfo(entities, data.eval);
  EvaluateResults(false);
}

function GroundTruthRequest() {
  $.ajax({url: server + "/ground-truth/",
    data: "0",
    dataType: "json",
    success: GroundTruthRequestCallback});
}

function GroundTruthRequestCallback(data, status, xhr) {
  ground_truth.queries = data.ground_truth.queries;
  ground_truth.entities = data.ground_truth.entities;
  ground_truth.keywords = data.ground_truth.keywords;
  ground_truth.target_keywords = data.ground_truth.target_keywords;
  ground_truth.coarse_types = data.ground_truth.coarse_types;
  ground_truth.valid = true;
  EvaluateResults(false);
}

function ScoreEntities(query, entities) {
  var ex_content_freq = [Number.MAX_VALUE, Number.MIN_VALUE];
  var ex_corpus_freq = [Number.MAX_VALUE, Number.MIN_VALUE];
  var ex_sfscore = [Number.MAX_VALUE, Number.MIN_VALUE];
  var ex_cfscore = [Number.MAX_VALUE, Number.MIN_VALUE];
  var ex_sdscore = [Number.MAX_VALUE, Number.MIN_VALUE];
  var ex_cdscore = [Number.MAX_VALUE, Number.MIN_VALUE];
  var scores = [];
  for (var i = 0; i < entities.length; ++i) {
    var name = entities[i][0];
    var type = entities[i][1];
    var content_freq = entities[i][2];
    var snippet_freq = entities[i][3];
    var corpus_freq = entities[i][4];
    var score = Score(query, entities[i]);
    scores.push(score);
    var doc_freq = entities[i][6].length;
    ex_content_freq[0] = Math.min(ex_content_freq[0], content_freq);
    ex_content_freq[1] = Math.max(ex_content_freq[1], content_freq);
    ex_corpus_freq[0] = Math.min(ex_corpus_freq[0], corpus_freq);
    ex_corpus_freq[1] = Math.max(ex_corpus_freq[1], corpus_freq);
    ex_cfscore[0] = Math.min(ex_cfscore[0], score.cf);
    ex_cfscore[1] = Math.max(ex_cfscore[1], score.cf);
    ex_sfscore[0] = Math.min(ex_sfscore[0], score.sf);
    ex_sfscore[1] = Math.max(ex_sfscore[1], score.sf);
    ex_cdscore[0] = Math.min(ex_cdscore[0], score.cdf);
    ex_cdscore[1] = Math.max(ex_cdscore[1], score.cdf);
    ex_sdscore[0] = Math.min(ex_sdscore[0], score.sdf);
    ex_sdscore[1] = Math.max(ex_sdscore[1], score.sdf);
  }
  var cfw = scoring_options.cfw[10];
  var sfw = scoring_options.sfw[10];
  var cdfw = scoring_options.cdfw;
  var sdfw = scoring_options.sdfw;
  for (var i = 0; i < entities.length; ++i) {
    entities[i][5] = 0.0;
    entities[i][8] = scores[i].filtered;
    if (scores[i].filtered) {
      // continue;
    }
    var value = scores[i].cf / ex_cfscore[1] * cfw +
                scores[i].sf / ex_sfscore[1] * sfw +
                scores[i].cdf / ex_cdscore[1] * cdfw +
                scores[i].sdf / ex_sdscore[1] * sdfw;
    entities[i][5] += value;
    if (!scores[i].filtered) {
      var type = entities[i][1];
      if (search_result.type_scores[type] === undefined) {
        search_result.type_scores[type] = value;
      } else {
        search_result.type_scores[type] += value;
      }
    }
  }
}

function SortEntities(entities, i) {
  if (i == undefined) {
    i = 5;
  }
  entities.sort(function (a, b) {
    return b[i] - a[i];
  });
}

function FilterEntities(entities) {
  if (scoring_options.coarse_type_filter) {
    var k = Math.min(10, entities.length);
    var type_scores = {misc: -0.1};
    var best_type = [Number.MIN_VALUE, "unknown"];
    for (var i = 0; i < k; ++i) {
      var entity = entities[i];
      var name = entity[0];
      var type = entity[1];
      var score = entity[5];
      var filtered = entity[8];
      if (filtered) {
        k = Math.min(k + 1, entities.length);
        continue;
      }
      var ts = type_scores[type];
      if (type_scores[type] == undefined) {
        type_scores[type] = 0;
      }
      type_scores[type] += Math.log(score + 1) * (score + 0.5);
      if (type_scores[type] > best_type[0]) {
        best_type[0] = type_scores[type];
        best_type[1] = type;
      }
    }
    for (var i = 0; i < k; ++i) {
      var entity = entities[i];
      var name = entity[0];
      var type = entity[1];
      var filtered = entity[8];
      if (filtered) {
        continue;
      }
      if (type != best_type[1]) {
        entity[8] = true;
      }
    }
  }
}

function ExpMovAvg(scores, k) {
  k = Math.min(k, scores.length);
  var a = 2.0 / (k + 1);
  var b = scores.length - k;
  var ema = scores[b];
  for (var i = b - 1; i >= 0; --i) {
    ema = a * scores[i] + (1 - a) * ema;
  }
  return ema;
}

function UpdateSemanticEntityChart(entities) {
  var k = Math.min(20, entities.length);
  var array = [["Entity", "Score"]];

  if (k == 0) {
    array.push(["", 0]);
  }
  for (var i = 0; i < k; ++i) {
    var name = entities[i].inst;
    var beg = name.lastIndexOf(":") + 1;
    var end = name.indexOf("(") - 1;
    if (end == -2) {
      end = name.length;
    }
    name = name.substr(beg, end - beg).ReplaceAll("_", " ");
    var score = parseInt(entities[i].score);
    array.push([name.toUpperCase(), score]); 
  }
  var data = google.visualization.arrayToDataTable(array);
  var options = {
    backgroundColor: {fill: "transparent", stroke: "#f4f8f7", strokeWidth: 4},
    colors: ["#f4f8f7"],
    fontName: "Lato",
    chartArea: {left:160, top:20, height:"88%"},
    legend: {textStyle: {color: "#f4f8f7"}},
    vAxis: {textStyle: {color: "#f4f8f7"}},
    hAxis: {textStyle: {color: "#f4f8f7"}}
  };

  var chart = new google.visualization.BarChart(
      document.getElementById("semantic-entity-chart"));
  chart.draw(data, options);
}

function ScoreCutoff(score, avg_score, min_score, max_score) {
  return score < avg_score * 0.7 || score < max_score * 0.3;
}

function UpdateEntityChart(entities) {
  var k = Math.min(20, entities.length);
  
  var ex_score = [Number.MAX_VALUE, Number.MIN_VALUE];
  var ex_content_freq = [Number.MAX_VALUE, Number.MIN_VALUE];
  var ex_corpus_freq = [Number.MAX_VALUE, Number.MIN_VALUE];
  var score_sum = 0;
  var scores = [];

  for (var i = 0; i < k; ++i) {
    var name = entities[i][0];
    var type = entities[i][1];
    var content_freq = entities[i][2];
    var snippet_freq = entities[i][3];
    var corpus_freq = entities[i][4];
    var score = entities[i][5];
    var doc_freq = entities[i][6].length;
    var filtered = entities[i][8];
    if (filtered) {
      k = Math.min(k + 1, entities.length);
      continue;
    }
    score_sum += score;
    scores.push(score);
    ex_score[0] = Math.min(ex_score[0], score);
    ex_score[1] = Math.max(ex_score[1], score);
    ex_content_freq[0] = Math.min(ex_content_freq[0], content_freq);
    ex_content_freq[1] = Math.max(ex_content_freq[1], content_freq);
    ex_corpus_freq[0] = Math.min(ex_corpus_freq[0], corpus_freq);
    ex_corpus_freq[1] = Math.max(ex_corpus_freq[1], corpus_freq);
  }

  var array = [["Entity", "Content Freq",
        "Snippet Freq", "Corpus Freq (relative)",
        "Score (relative)"]];
  var score_div = ex_content_freq[1] / ex_score[1];
  var freq_div = ex_content_freq[1] / ex_corpus_freq[1];
  // var avg_score = score_sum / k;
  var avg_score = ExpMovAvg(scores, 10);
  for (var i = 0; i < k; ++i) {
    var name = entities[i][0];
    var type = entities[i][1];
    var content_freq = entities[i][2];
    var snippet_freq = entities[i][3];
    var corpus_freq = entities[i][4];
    var score = entities[i][5];
    var doc_freq = entities[i][6].length;
    var filtered = entities[i][8];
    if (filtered) {
      continue;
    }
    if (ScoreCutoff(score, avg_score, ex_score[0], ex_score[1])) {
      break;
    }
    array.push([name.toUpperCase(), content_freq, snippet_freq,
            corpus_freq * freq_div,
            score * score_div]); 
  }
  var data = google.visualization.arrayToDataTable(array);
  var options = {
    backgroundColor: {fill: "transparent", stroke: "#f4f8f7", strokeWidth: 4},
    colors: ["#f4f8f7", "#d0d6aa", "#51bab6", "#c93a3e"],
    fontName: "Lato",
    chartArea: {left:160, top:20, height:"88%"},
    legend: {textStyle: {color: "#f4f8f7"}},
    vAxis: {textStyle: {color: "#f4f8f7"}},
    hAxis: {textStyle: {color: "#f4f8f7"}}
  };

  var chart = new google.visualization.BarChart(
      document.getElementById("entity-chart"));
  chart.draw(data, options);
}

function UpdateOptions(elem, show) {
  if (elem.indexOf("Performance") != -1) {
    options.show_performance = show;
    return;
  }
  if (elem.indexOf("Query Analysis") != -1) {
    options.show_query_analysis = show;
    return;
  }
  if (elem.indexOf("Target Types") != -1) {
    options.show_target_types = show;
    return;
  }
  if (elem.indexOf("Semantic Query") != -1) {
    options.show_semantic_query = show;
    return;
  }
  if (elem.indexOf("Semantic Entity Chart") != -1) {
    options.show_semantic_entity_chart = show;
    return;
  }
  if (elem.indexOf("Scoring") != -1) {
    options.show_scoring = show;
    return;
  }
  if (elem.indexOf("Entity Selection Chart") != -1) {
    options.show_entity_chart = show;
    return;
  }
  if (elem.indexOf("Entity Table") != -1) {
    options.show_entity_table = show;
    return;
  }
  if (elem.indexOf("Evaluation") != -1) {
    options.show_evaluation = show;
    return;
  }
  if (elem.indexOf("Documents") != -1) {
    options.show_documents = show;
    return;
  }
}

function UseOptions() {
  if (options.show_performance) {
    $("#performance-toggle").click();
  }
  if (options.show_query_analysis) {
    $("#query-analysis-toggle").click();
  }
  if (options.show_target_types) {
    $("#semantic-analysis-toggle").click();
  }
  if (options.show_semantic_query) {
    $("#semantic-query-toggle").click();
  }
  if (options.show_semantic_entity_chart) {
    $("#semantic-entity-chart-toggle").click();
  }
  if (options.show_scoring) {
    $("#scoring-toggle").click();
  }
  if (options.show_entity_chart) {
    $("#entity-chart-toggle").click();
  }
  if (options.show_entity_table) {
    $("#entity-table-toggle").click();
  }
  if (options.show_evaluation) {
    $("#evaluation-toggle").click();
  }
  if (options.show_documents) {
    $("#documents-toggle").click();
  }
}

function LoadCookie(name) {
  if ($.cookie(name) == undefined ||
      $.cookie(name).v == undefined ||
      $.cookie(name).v < window[name].v) {
    $.cookie(name, window[name]);
  }
  window[name] = $.cookie(name);
}

function SaveSessionItem(name) {
  sessionStorage.setItem(name, JSON.stringify(window[name]));
}

function LoadSessionItem(name) {
  var item = JSON.parse(sessionStorage.getItem(name));
  if (item === null || item.v === undefined || item.v < window[name].v) {
    SaveSessionItem(name);
    item = JSON.parse(sessionStorage.getItem(name));
  }
  window[name] = item;
}

$(document).ready(
  function() {
    $.cookie.json = true;
    LoadCookie("options");
    LoadCookie("scoring_options");
    LoadSessionItem("evaluation");
    UseOptions();
    InitSliders();

    if (window.location.pathname == "/index.html") {
      window.location.pathname = "";
    }
    if (UrlQuery()) {
      var query = UserQuery(UserFormat(UrlQuery()));
      search_result = new SearchResult(query);
      Search(query);
    }
    EvaluateResults(false);
  }
);

$(document).keypress(
  function(event) {
    if (event.which == 13) {
      event.preventDefault();
      UrlQuery(UrlFormat(UserQuery()));
    } else if (event.charCode == 58) {
    }
  }
);

$(document).on("click", ".accordion-toggle", 
  function() {
    $(this).html(
      function(i, old) {
        var pos = old.indexOf("+");
        if (pos == -1) {
          pos = old.indexOf("-");
        }
        if (pos == -1) {
          return old;
        }
        UpdateOptions(old, old.charAt(pos) == '+' ? true : false);
        $.cookie("options", options);
        return old.substr(0, pos) + (old.charAt(pos) == '-' ? '+' : '-') +
            old.substr(pos + 1);
      }
    );
  }
);

google.load("visualization", "1", {packages:["corechart"]});
$.serverObserver.enable({
  url: server + "/index.html?" + (+new Date()),
  frequency: 15000,
  onServerOnline: function() {
    $("#server-status-area").css({"background-color": "#111e21"}); 
  },
  onServerOffline: function() {
    $("#server-status-area").css({"background-color": "#c93a3e"}); 
  }
});
