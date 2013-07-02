// Copyright 2013 Eugen Sawin <esawin@me73.com>
var server = "http://" + window.location.hostname + ":" + window.location.port;

var options = {
  v: "0.1.2",
  show_performance: false,
  show_query_analysis: false,
  show_target_types: false,
  show_semantic_query: false,
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

var entities = [];
var query = null;
var scoring_options = {
  v: "0.1.4",
  cfw: [0.6, 0.58, 0.55, 0.53, 0.5, 0.48, 0.45, 0.43, 0.4, 0.38, 0.30],
  sfw: [0.6, 0.58, 0.55, 0.53, 0.5, 0.48, 0.45, 0.43, 0.4, 0.38, 0.30],
  cdfw: 0.25,
  sdfw: 0.25,
  ontology_filter: 1,
  similarity_filter: 1,
  coarse_type_filter: 1,
  yago_type_filter: 0,
  fb_type_filter: 0,
};

var ground_truth = {
  valid: false,
  data: [],
};

var evaluation = {
  valid: false,
  next: 0,
  recalls: [],
  precisions_10: [],
  precisions_r: [],
};

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

function QueryTypeInfo(entities) {
  var k = 10;
  var es = "";
  for (var i = 0; i < entities.length; ++i) {
    if (entities[i][8]) {
      continue;
    }
    if (k-- < 0) {
      break;
    }
    if (i > 0) {
      es += '+';
    }
    es += '"' + entities[i][0] + '"';
  }
  $.ajax({url: server + "/",
    data: "ti=" + es + "&" + ServerOptions(),
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

function PreFilter(query, entity) {
  if (scoring_options.ontology_filter &&
      entity[4] == 0) {
    return false;
  }
  if (scoring_options.similarity_filter) {
    var entity_words = entity[0].split(" ");
    var query_words = query.split(" ");
    var num_similar = 0;
    for (var j = 0; j < entity_words.length; ++j) {
      if (entity_words[j].length > 3) {
        continue;
      }
      for (var i = 0; i < query_words.length; ++i) {
        var k = 0;
        while (i + k < query_words.length &&
               entity_words[j][k] == query_words[i + k][0]) {
          ++k;
        }
        if (k == entity_words[j].length) {
          ++num_similar;
          break;
        }
      }
    }
    for (var i = 0; i < query_words.length; ++i) {
      for (var j = 0; j < entity_words.length; ++j) {
        var ped = 0;
        if (query_words[i].length > entity_words[j].length) {
          ped = PrefixEditDistance(entity_words[j], query_words[i]);
        } else {
          ped = PrefixEditDistance(query_words[i], entity_words[j]);
        }
        if (ped <= Math.min(query_words[i].length, entity_words[j].length) / 3) {
          ++num_similar;
        }
      }
    }
    if (num_similar > Math.min(query_words.length, entity_words.length) / 2) {
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
}

function UpdateEntityTable(entities) {
  var ex_score = [Number.MAX_VALUE, Number.MIN_VALUE];
  var ex_content_freq = [Number.MAX_VALUE, Number.MIN_VALUE];
  var ex_corpus_freq = [Number.MAX_VALUE, Number.MIN_VALUE];
  var entity_table = "<thead><tr><th>Entity</th><th>Coarse Type</th>" +
    "<th>Content Frequency</th>" +
    "<th>Snippet Frequency</th>" +
    "<th>Document Frequency</th>" +
    "<th>Corpus Frequency</th>" +
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
      "<td>" + score.toFixed(3) + "</td></tr>";
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
    var entity_score = 0;
    var entity_name = data.yago_types[i][0];
    var yago_types = data.yago_types[i][1];
    for (var j = i; j < entities.length; ++j) {
      if (entities[j][0] == entity_name) {
        entity_score = entities[j][5];
        break;
      }
    }
    for (var j = 0; j < yago_types.length; ++j) {
      if (// yago_types[j][1] > 2400000 ||
          yago_types[j][1] < 50 ||
          yago_types[j][0].length > 18) {
        continue;
      }
      if (type_scores[yago_types[j][0]] == undefined) {
        type_scores[yago_types[j][0]] = [0, yago_types[j][1]];
      }
      var itf = 1 / (yago_types[j][1] + 10000) / Math.log(yago_types[j][1] + 100);
      type_scores[yago_types[j][0]][0] += entity_score * itf;
      var score = type_scores[yago_types[j][0]][0];
      if (score > best_type[0]) {
        best_type[0] = score;
        best_type[1] = yago_types[j][0];
      }
    }
  }
  // console.log(type_scores);
  console.log(best_type);
}

function SearchCallback(data, status, xhr) {
  if (data.eval !== undefined) {
    UpdateEvaluation(data);
    return;
  }
  var durations = [["Procedure", "Duration [ms]"],
      // ["Total", data.duration / 1000],
      ["Query Analysis", data.query_analysis.duration / 1000],
      ["Document Retrieval", data.document_retrieval.duration / 1000],
      ["Entity Extraction", data.entity_extraction.duration / 1000],
      ["Entity Ranking", data.entity_ranking.duration / 1000],
      ["Semantic Query Construction", data.semantic_query.duration / 1000]];
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

  entities = data.entity_extraction.entity_items;
  ScoreEntities(query, entities);
  SortEntities(entities);
  FilterEntities(entities);
  UpdateEntityTable(entities);
  UpdateEntityChart(entities);
  UpdateSemanticQuery(entities);

  var broccoli_query = "";
  broccoli_query += data.semantic_query.broccoli_query;
  $("#broccoli-query-area").html(broccoli_query);

  var yago_target_types = "";
  for (var i in data.semantic_query.target_types) {
    var type = data.semantic_query.target_types[i][0];
    var score = data.semantic_query.target_types[i][1];
    var total_freq = data.semantic_query.target_types[i][2];
    yago_target_types += "<span class=\"label\">" + type.toUpperCase() + "</span> ";
  }
  $("#yago-target-types").html(yago_target_types);
  var fb_target_types = "";
  for (var i in data.semantic_query.fb_target_types) {
    var type = data.semantic_query.fb_target_types[i][0];
    var score = data.semantic_query.fb_target_types[i][1];
    var total_freq = data.semantic_query.fb_target_types[i][2];
    fb_target_types += "<span class=\"label\">" + type.toUpperCase() + "</span> ";
  }
  $("#fb-target-types").html(fb_target_types);
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
  if (init == undefined) {
    init = true;
    evaluation.valid = false;
    evaluation.next = 0;
  }
  if (!ground_truth.valid) {
    GroundTruthRequest();
  } else if (evaluation.valid) {
    return;
  } else {
    if (evaluation.next >= ground_truth.data.length) {
      evaluation.valid = true;
      evaluation.next = 0;
      ApplySortability();
    } else {
      Search(ground_truth.data[evaluation.next][0], "&eval=" + evaluation.next);
      ++evaluation.next;
    }
  }
}

function UpdateEvaluation(data) {
  var entities = data.entity_extraction.entity_items;
  var query = data.query_analysis.keywords.slice(0).join(" ");
  ScoreEntities(query, entities);
  SortEntities(entities);
  var relevant = {};
  var num_rel = ground_truth.data[data.eval].length - 1;
  var recall = 0;
  var precision_10 = 0;
  var precision_r = 0;
  for (var i = 1; i < num_rel + 1; ++i) {
    relevant[ground_truth.data[data.eval][i]] = 0;
  }
  for (var i = 0; i < entities.length; ++i) {
    var name = entities[i][0];
    var filtered = entities[i][8];
    if (!filtered && relevant[name] == 0) {
      relevant[name] = i + 1;
      ++recall;
      if (i < 10) {
        ++precision_10;
      }
      if (i < num_rel) {
        ++precision_r;
      }
    }
  }
  recall /= num_rel;
  precision_10 /= 10;
  precision_r /= num_rel;
  evaluation.recalls[data.eval] = recall;
  evaluation.precisions_10[data.eval] = precision_10;
  evaluation.precisions_r[data.eval] = precision_r;
  var table_header = "<thead><tr>" +
    "<th>Id</th>" +
    "<th>Query</th>" +
    "<th>Recall</th>" +
    "<th>P@10</th>" +
    "<th>P@R</th>" +
    "</tr></thead><tbody>";
  var table = $("#evaluation-table").html();
  if (data.eval == 0) {
    table = table_header;
  } else if (data.eval == ground_truth.data.length - 1) {
    var num = ground_truth.data.length;
    var avg_recall = evaluation.recalls.reduce(
        function(v1, v2) { return v1 + v2; }
    ) / num;
    var avg_precision_10 = evaluation.precisions_10.reduce(
        function(v1, v2) { return v1 + v2; } 
    ) / num;
    var avg_precision_r = evaluation.precisions_r.reduce(
        function(v1, v2) { return v1 + v2; }
    ) / num;
    table = table_header +
      "<tr class=\"error\">" + "<td>0</td>" + 
      "<td>MEAN</td>" +
      "<td>" + avg_recall.toFixed(3) + "</td>" +
      "<td>" + avg_precision_10.toFixed(3) + "</td>" +
      "<td>" + avg_precision_r.toFixed(3) + "</td>" +
      "</tr>" + table.substr(table_header.length);
  } else {
    table = table.substr(0, table.length - 8);
  }
  var query = ground_truth.data[data.eval][0];
  table += "<tr><td>" + (data.eval + 1) + "</td>" + 
    '<td><a href=\'' + server + '/?q="' + query + '"\'>' + query  + "</a></td>" +
    "<td>" + recall.toFixed(3) + "</td>" +
    "<td>" + precision_10.toFixed(3) + "</td>" +
    "<td>" + precision_r.toFixed(3) + "</td>" +
    "</tr>";
  table += "</tbody>";
  $("#evaluation-table").html(table);
  EvaluateResults(false);
}

function GroundTruthRequest() {
  $.ajax({url: server + "/ground-truth/",
    data: "0",
    dataType: "json",
    success: GroundTruthRequestCallback});
}

function GroundTruthRequestCallback(data, status, xhr) {
  ground_truth.data = data.ground_truth;
  ground_truth.valid = true;
  EvaluateResults();
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
    entities[i][5] += scores[i].cf / ex_cfscore[1] * cfw +
                      scores[i].sf / ex_sfscore[1] * sfw +
                      scores[i].cdf / ex_cdscore[1] * cdfw +
                      scores[i].sdf / ex_sdscore[1] * sdfw;
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
  var a = 2.0 / (k + 1);
  var b = scores.length - k;
  var ema = scores[b];
  for (var i = b - 1; i >= 0; --i) {
    ema = a * scores[i] + (1 - a) * ema;
  }
  return ema;
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

  var array = [["Entity", "Content Frequency",
        "Snippet Frequency", "Corpus Frequency (relative)",
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
    if (score < avg_score * 0.7 || score < ex_score[1] * 0.3) {
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
  if (elem.indexOf("Scoring") != -1) {
    options.show_scoring = show;
    return;
  }
  if (elem.indexOf("Entity Chart") != -1) {
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

$(document).load(
  function() {
  }
);

$(document).ready(
  function() {
    $.cookie.json = true;
    if (!$.cookie("pythia_options") ||
        $.cookie("pythia_options").v == undefined ||
        $.cookie("pythia_options").v < options.v) {
      $.cookie("pythia_options", options);
    }
    if (!$.cookie("scoring_options") ||
        $.cookie("scoring_options").v == undefined ||
        $.cookie("scoring_options").v < scoring_options.v) {
      $.cookie("scoring_options", scoring_options);
    }
    options = $.cookie("pythia_options");
    scoring_options = $.cookie("scoring_options");
    UseOptions();
    InitSliders();

    if (window.location.pathname == "/index.html") {
      window.location.pathname = "";
    }
    if (UrlQuery()) {
      Search(UserQuery(UserFormat(UrlQuery())));
    }
    EvaluateResults();
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
        $.cookie("pythia_options", options);
        return old.substr(0, pos) + (old.charAt(pos) == '-' ? '+' : '-') +
            old.substr(pos + 1);
      }
    );
  }
);

google.load("visualization", "1", {packages:["corechart"]});
$.serverObserver.enable({
  url: server + "/index.html?" + (+new Date()),
  frequency: 5000,
  onServerOnline: function() {
    $("#server-status-area").css({"background-color": "#111e21"}); 
  },
  onServerOffline: function() {
    $("#server-status-area").css({"background-color": "#c93a3e"}); 
  }
});
