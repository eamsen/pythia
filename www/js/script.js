var server = "http://" + window.location.hostname + ":" + window.location.port;

var options = {
  "show_performance": false,
  "show_query_analysis": false,
  "show_target_types": false,
  "show_semantic_query": false,
  "show_entity_chart": false,
  "show_entity_table": false,
  "show_documents": false
};

var server_options = {
  // Freebase target type detection.
  "fbtt": 1
};

function serverOptions() {
  var o = "";
  for (var i in server_options) {
    if (o.length > 0) {
      o += "&";
    }
    o += i + "=" + server_options[i];
  }
  return o;
}

function urlFormat(q) {
  return q.replace(/ /g, '+');
}

function userFormat(q) {
  return q.replace(/\+/g, ' ');
}

function userQuery(q) {
  if (q) {
    document.getElementById("query").value = q.toLowerCase(); 
  }
  return document.getElementById('query').value; 
}

function urlQuery(q) {
  if (q) {
    window.location.search = '?q=' + q.toLowerCase();
  }
  return window.location.search.substr(3);
}

function search() {
  userQuery(userFormat(urlQuery()));
  $.ajax({url: server + "/",
    data: "qf=" + urlQuery() + "&" + serverOptions(),
    dataType: "json",
    success: callback});
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

  var max_score = 0;
  var max_content_freq = 0;
  var max_entity_freq = 0;
  var entity_table = "<thead><tr><th>Entity</th><th>Coarse Type</th>" +
    "<th>Content Frequency</th>" +
    "<th>Snippet Frequency</th>" +
    "<th>Entity Frequency</th>" +
    "<th>Score</th>" +
    "</tr></thead><tbody>";
  for (var i in data.entities) {
    var entity = data.entities[i][0].split(":");
    var content_freq = data.entities[i][1][0];
    var snippet_freq = data.entities[i][1][1];
    var score = data.entities[i][1][2];
    var entity_freq = data.entities[i][1][3];
    if (entity_freq > 0) {
      entity_table += "<tr>";
    } else {
      entity_table += "<tr class=\"error\">";
    }
    entity_table += "<td>" + entity[0].toUpperCase() + "</td>" +
      "<td>" + entity[1] + "</td>" +
      "<td>" + content_freq + "</td>" +
      "<td>" + snippet_freq + "</td>" +
      "<td>" + entity_freq + "</td>" +
      "<td>" + score + "</td></tr>";
    max_score = Math.max(max_score, score);
  }
  for (var i in data.top_entities) {
    var entity = data.top_entities[i][0];
    var content_freq = data.top_entities[i][1];
    var snippet_freq = data.top_entities[i][2];
    var score = data.top_entities[i][3];
    var entity_freq = data.top_entities[i][4];
    max_content_freq = Math.max(max_content_freq, content_freq);
    max_entity_freq = Math.max(max_entity_freq, entity_freq);
  }
  entity_table += "</tbody>";
  $("#entity-table").html(entity_table);
  ApplySortability();

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
  drawEntityChart(data, max_score, max_entity_freq, max_content_freq);
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

function drawEntityChart(data, max_score, max_entity_freq, max_content_freq) {
  var array = [["Entity", "Content Frequency",
        "Snippet Frequency", "Entity Frequency (relative)",
        "Score (relative)"]];
  var score_div = max_content_freq / max_score;
  var freq_div = max_content_freq / max_entity_freq;
  for (var i in data.top_entities) {
    var entity = data.top_entities[i][0];
    var content_freq = data.top_entities[i][1];
    var snippet_freq = data.top_entities[i][2];
    var score = data.top_entities[i][3];
    var entity_freq = data.top_entities[i][4];
    if (score < max_score * 0.1) {
      continue;
    }
    array.push([entity.toUpperCase(), content_freq, snippet_freq,
            entity_freq * freq_div,
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
    options = $.cookie("pythia_options");
    UseOptions();

    if (window.location.pathname == "/index.html") {
      window.location.pathname = "";
    }
    if (urlQuery()) {
      search();
    }
  }
);

$(document).keypress(
  function(event) {
    if (event.which == 13) {
      event.preventDefault();
      urlQuery(urlFormat(userQuery()));
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
    $("#status-area").css({"background-color": "#111e21"}); 
  },
  onServerOffline: function() {
    $("#status-area").css({"background-color": "#c93a3e"}); 
  }
});
