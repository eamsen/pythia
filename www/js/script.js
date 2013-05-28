var server = "http://" + window.location.hostname + ":" + window.location.port;

var options = {
  show_performance: false,
  show_query_analysis: false,
  show_target_types: false,
  show_semantic_query: false,
  show_scoring: false,
  show_entity_chart: false,
  show_entity_table: false,
  show_documents: false
};

var server_options = {
  // Freebase target type detection.
  fbtt: 1
};

var entities = [];
var scoring_options = {
  cfw: [0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5],
  sfw: [0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5],
  cdfw: 0.5,
  sdfw: 0.5
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
  return document.getElementById('query').value; 
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
    UpdateEntityTable();
    UpdateEntityChart();
  };
}

function SetSfw(v) {
}

function InitSliders() {
  for (var i = 0; i < 10; ++i) {
    InitVSlider("cfw" + i, scoring_options.cfw[i], SetOptionFunc("cfw", i));
    InitVSlider("sfw" + i, scoring_options.cfw[i], SetOptionFunc("sfw", i));
  }
  InitVSlider("cdfw", scoring_options.cdfw, SetOptionFunc("cdfw"));
  InitVSlider("sdfw", scoring_options.sdfw, SetOptionFunc("sdfw"));
}

function Search() {
  UserQuery(UserFormat(UrlQuery()));
  $.ajax({url: server + "/",
    data: "qf=" + UrlQuery() + "&" + ServerOptions(),
    dataType: "json",
    success: callback});
}

function Score(entity) {
  var content_freq = entity[2];
  var snippet_freq = entity[3];
  var corpus_freq = entity[4];
  var content_freqs = entity[6];
  var snippet_freqs = entity[7];
  var content_doc_freq = content_freqs.length;
  var snippet_doc_freq = snippet_freqs.length;
  var corpus_if = corpus_freq > 0 ? 24.0 - Math.log(10.0 + corpus_freq) : 0.0;
  // var w = [0.0, 1.0, 25.0, 0.0, 10.0, 100.0];
  var cfw = scoring_options.cfw;
  var sfw = scoring_options.sfw;
  var cdfw = scoring_options.cdfw;
  var sdfw = scoring_options.sdfw;
  var corfw = scoring_options.corfw;

  var score = 0;
  for (var i in content_freqs) {
    score += cfw[content_freqs[i][0]] * content_freqs[i][1];
  }
  for (var i in snippet_freqs) {
    score += cfw[snippet_freqs[i][0]] * snippet_freqs[i][1];
  }
  score += cdfw * content_doc_freq;
  score += sdfw * snippet_doc_freq;
  return score;
}

function UpdateEntityTable() {
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
    var score = Score(entities[i]);
    entities[i][5] = score;
    var doc_freq = entities[i][6].length;
    if (corpus_freq > 0) {
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
      "<td>" + score + "</td></tr>";
  }
  entity_table += "</tbody>";
  $("#entity-table").html(entity_table);
  ApplySortability();
}

function callback(data, status, xhr) {
  var durations = [["Procedure", "Duration [ms]"],
      // ["Total", data.duration / 1000],
      ["Query Analysis", data.query_analysis.duration / 1000],
      ["Document Retrieval", data.document_retrieval.duration / 1000],
      ["Entity Extraction", data.entity_extraction.duration / 1000],
      ["Entity Ranking", data.entity_ranking.duration / 1000],
      ["Semantic Query Construction", data.semantic_query.duration / 1000]];
  drawPerformanceChart(durations, data.duration / 1000);
  var target_keywords = {};
  for (var i in data.query_analysis.target_keywords) { 
    var keywords = data.query_analysis.target_keywords[i].split(" ");
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
  UpdateEntityTable();
  UpdateEntityChart();

  var ex_score = [Number.MAX_VALUE, Number.MIN_VALUE];
  var ex_content_freq = [Number.MAX_VALUE, Number.MIN_VALUE];
  var ex_corpus_freq = [Number.MAX_VALUE, Number.MIN_VALUE];

  for (var i in data.top_entities) {
    var entity = data.top_entities[i][0];
    var content_freq = data.top_entities[i][1];
    var snippet_freq = data.top_entities[i][2];
    var score = data.top_entities[i][3];
    var corpus_freq = data.top_entities[i][4];
    ex_content_freq[0] = Math.min(ex_content_freq[0], content_freq);
    ex_corpus_freq[0] = Math.min(ex_corpus_freq[0], corpus_freq);
    ex_score[0] = Math.min(ex_score[0], score);
    ex_content_freq[1] = Math.max(ex_content_freq[1], content_freq);
    ex_corpus_freq[1] = Math.max(ex_corpus_freq[1], corpus_freq);
    ex_score[1] = Math.max(ex_score[1], score);
  }

  var broccoli_query = "";
  broccoli_query += data.semantic_query.broccoli_query;
  $("#broccoli-query-area").html(broccoli_query);

  var yago_target_types = "";
  for (var i in data.semantic_query.target_types) {
    var type = data.semantic_query.target_types[i][0];
    var score = data.semantic_query.target_types[i][1];
    var total_freq = data.semantic_query.target_types[i][2];
    if (i > 0) {
      yago_target_types += ", ";
    }
    yago_target_types += "<span class=\"label\">" + type.toUpperCase() + "</span>";
  }
  $("#yago-target-types").html(yago_target_types);
  var fb_target_types = "";
  for (var i in data.semantic_query.fb_target_types) {
    var type = data.semantic_query.fb_target_types[i][0];
    var score = data.semantic_query.fb_target_types[i][1];
    var total_freq = data.semantic_query.fb_target_types[i][2];
    if (i > 0) {
      fb_target_types += ", ";
    }
    fb_target_types += "<span class=\"label\">" + type.toUpperCase() + "</span>";
  }
  $("#fb-target-types").html(fb_target_types);
  // drawEntityChart(data, ex_score, ex_corpus_freq, ex_content_freq);
}

function drawPerformanceChart(durations, total) {
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

function UpdateEntityChart() {
  entities.sort(function (a, b) {
    return b[5] - a[5];
  });
  var k = Math.min(20, entities.length);
  
  var ex_score = [Number.MAX_VALUE, Number.MIN_VALUE];
  var ex_content_freq = [Number.MAX_VALUE, Number.MIN_VALUE];
  var ex_corpus_freq = [Number.MAX_VALUE, Number.MIN_VALUE];

  for (var i = 0; i < k; ++i) {
    var name = entities[i][0];
    var type = entities[i][1];
    var content_freq = entities[i][2];
    var snippet_freq = entities[i][3];
    var corpus_freq = entities[i][4];
    var score = entities[i][5];
    var doc_freq = entities[i][6].length;
    ex_content_freq[0] = Math.min(ex_content_freq[0], content_freq);
    ex_corpus_freq[0] = Math.min(ex_corpus_freq[0], corpus_freq);
    ex_score[0] = Math.min(ex_score[0], score);
    ex_content_freq[1] = Math.max(ex_content_freq[1], content_freq);
    ex_corpus_freq[1] = Math.max(ex_corpus_freq[1], corpus_freq);
    ex_score[1] = Math.max(ex_score[1], score);
  }

  var array = [["Entity", "Content Frequency",
        "Snippet Frequency", "Entity Frequency (relative)",
        "Score (relative)"]];
  var score_div = ex_content_freq[1] / ex_score[1];
  var freq_div = ex_content_freq[1] / ex_corpus_freq[1];
  for (var i = 0; i < k; ++i) {
    var name = entities[i][0];
    var type = entities[i][1];
    var content_freq = entities[i][2];
    var snippet_freq = entities[i][3];
    var corpus_freq = entities[i][4];
    var score = entities[i][5];
    var doc_freq = entities[i][6].length;
    if (score < ex_score[1] * 0.2) {
      continue;
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

function drawEntityChart(data, ex_score, ex_corpus_freq, ex_content_freq) {
  var array = [["Entity", "Content Frequency",
        "Snippet Frequency", "Entity Frequency (relative)",
        "Score (relative)"]];
  var score_div = ex_content_freq[1] / ex_score[1];
  var freq_div = ex_content_freq[1] / ex_corpus_freq[1];
  for (var i in data.top_entities) {
    var entity = data.top_entities[i][0];
    var content_freq = data.top_entities[i][1];
    var snippet_freq = data.top_entities[i][2];
    var score = data.top_entities[i][3];
    var corpus_freq = data.top_entities[i][4];
    if (score < ex_score[1] * 0.2) {
      continue;
    }
    array.push([entity.toUpperCase(), content_freq, snippet_freq,
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
    if (!$.cookie("pythia_options")) {
      $.cookie("pythia_options", options);
    }
    if (!$.cookie("scoring_options")) {
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
      Search();
    }
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
  frequency: 3000,
  onServerOnline: function() {
    $("#server-status-area").css({"background-color": "#111e21"}); 
  },
  onServerOffline: function() {
    $("#server-status-area").css({"background-color": "#c93a3e"}); 
  }
});
