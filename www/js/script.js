var server = "http://" + window.location.hostname + ":" + window.location.port;

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
    data: "qf=" + urlQuery(),
    dataType: "json",
    success: callback});
}

function callback(data, status, xhr) {
  console.log(data);
  var target_keywords = {};
  for (var i in data.query_analysis.target_keywords) { 
    var keyword = data.query_analysis.target_keywords[i];
    target_keywords[keyword] = true;
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
        "<span class=\"label label-info\">" + word + "</span></a>";
    } else if (word in keywords) {
      query_analysis += "<a href=\"#\" data-placement=\"top\" " +
        "data-toggle=\"tooltip\" title=\"keyword\">" +
        "<span class=\"label label-success\">" + word + "</span></a>";
    } else {
      query_analysis += "<a href=\"#\" data-placement=\"top\" " +
        "data-toggle=\"tooltip\" title=\"no keyword\">" +
        "<span class=\"label\">" + word + "</span></a>";
    }
    query_analysis += " ";
  } 
  $("#query-analysis-area").html(query_analysis);

  var view_left = "";
  for (var i in data.results) {
    var title = data.results[i]["htmlTitle"];
    var snippet = data.results[i]["snippet"];
    var link = data.results[i]["link"];
    var link_name = data.results[i]["displayLink"]
    var element = "<p>" + "<a href=\"" + link + "\"><h8>" + title  + "</h8></a>"
        + "<br><span class=\"document-snippet\">" + snippet + "</span><br>"
        + "<a href=\"" + link + "\">" + link_name + "</a></p>";
    view_left += element;
  }
  if (data.results.length == 0) {
    // view_left += "<h2>no results</h2>";
  }
  $("#result-area").html(view_left);

  var max_content_freq = 0;
  var entity_table = "<thead><tr><th>Entity</th><th>Coarse Type</th>" +
    "<th>Content Frequency</th><th>Snippet Frequency</th></tr></thead><tbody>";
  for (var i in data.entity_extraction) {
    var entity = data.entity_extraction[i][0].split(":");
    var content_freq = data.entity_extraction[i][1][0];
    var snippet_freq = data.entity_extraction[i][1][1];
    var in_ontology = data.entity_extraction[i][1][2];
    if (in_ontology) {
      entity_table += "<tr>";
    } else {
      entity_table += "<tr class=\"error\">";
    }
    entity_table += "<td>" + entity[0] + "</td>" +
      "<td>" + entity[1] + "</td>" +
      "<td>" + content_freq + "</td>" +
      "<td>" + snippet_freq + "</td></tr>";
    max_content_freq = Math.max(max_content_freq, content_freq);
  }
  entity_table += "</tbody>";
  $("#entity-table").html(entity_table);
  ApplySortability();

  var broccoli_query = "";
  broccoli_query += data.broccoli_query;
  $("#broccoli-query-area").html(broccoli_query);

  var semantic_analysis = "";
  for (var i in data.target_types) {
    var type = data.target_types[i];
    if (i > 0) {
      semantic_analysis += ", ";
    }
    semantic_analysis += "<span class=\"label\">" + type + "</span>";
  }
  $("#semantic-analysis-area").html(semantic_analysis);
  drawChart(data, max_content_freq);
}

function drawChart(data, max_content_freq) {
  var array = new Array(new Array("Entity", "Content Frequency",
        "Snippet Frequency", "Base Score", "Score"));
  for (var i in data.entity_extraction) {
    var entity = data.entity_extraction[i][0].split(":");
    var content_freq = data.entity_extraction[i][1][0];
    var snippet_freq = data.entity_extraction[i][1][1];
    var in_ontology = data.entity_extraction[i][1][2];
    var base_score = 200;
    var score = 100;
    if (content_freq > max_content_freq * 0.2) {
      array.push(new Array(entity[0], content_freq, snippet_freq, base_score,
            score)); 
    }
  }
  var data = google.visualization.arrayToDataTable(array);

  var options = {
    backgroundColor: "transparent",
    colors: ["#f4f8f7", "#d0d6aa", "#51bab6", "#c93a3e"],
    fontName: "Lato",
    chartArea: {left:120, top:0, height:"95%"},
    legend: {textStyle: {color: "#f4f8f7"}},
    vAxis: {textStyle: {color: "#f4f8f7"}},
    hAxis: {textStyle: {color: "#f4f8f7"}}
  };

  var chart = new google.visualization.BarChart(
      document.getElementById("entity-graph"));
  chart.draw(data, options);
}

$(document).ready (
  function() {
    if (window.location.pathname == "/index.html") {
      window.location.pathname = "";
    }
    if (urlQuery()) {
      search();
    }
  }
);

$(document).keypress (
  function(event) {
    if (event.which == 13) {
      event.preventDefault();
      urlQuery(urlFormat(userQuery()));
    } else if (event.charCode == 58) {
    }
  }
);
google.load("visualization", "1", {packages:["corechart"]});
