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

  var entity_table = "<thead><tr><th>Entity</th><th>Content Frequency</th>" +
    "<th>Snippet Frequency</th></tr></thead>" +
      "<tbody>";
  for (var i in data.entity_extraction) {
    var entity = data.entity_extraction[i][0];
    var content_freq = data.entity_extraction[i][1][0];
    var snippet_freq = data.entity_extraction[i][1][1];
    entity_table += "<tr><td>" + entity + "</td><td>" +
      content_freq + "</td><td>" + snippet_freq + "</td></tr>";
  }
  entity_table += "</tbody>";
  $("#entity-table").html(entity_table);
  ApplySortability();

  var broccoli_query = "<div id=\"broccoli-query-area\">";
  broccoli_query += data.broccoli_query;
  broccoli_query += "</div>";
  $("#broccoli-query-area").replaceWith(broccoli_query);

  var meta_result1 = "<div id=\"meta-result-area1\">";
  for (var i in data.query_analysis.target_keywords) {
    var keyword = data.target_keywords[i];
    if (i > 0) {
      meta_result1 += ", ";
    }
    meta_result1 += keyword;
  }
  meta_result1 += " <span style=\"font-size: 0.61em;\">/</span> ";
  for (var i in data.target_types) {
    var type = data.target_types[i];
    if (i > 0) {
      meta_result1 += ", ";
    } else {
      // meta_result1 += "<h2>";
    }
    meta_result1 += type;
    if (i == 0) {
      // meta_result1 += "</h2>";
    }
  }
  meta_result1 += "</div>";
  $("#meta-result-area1").replaceWith(meta_result1);

  var meta_result2 = "<div id=\"meta-result-area2\">";
  for (var i in data.entities) {
    var name = data.entities[i]["name"];
    var type = data.entities[i]["type"];
    var rank = data.entities[i]["score"];
    // var element = "<h2>" + name + "</h2>:" + type + ":" + rank + "   ";
    var element = name + ":" + type + ":" + rank + " ";
    meta_result2 += element;
  }
  if (data.entities.length == 0) {
    meta_result2 += "no entities found";
  }
  meta_result2 += "</div>";
  $("#meta-result-area2").replaceWith(meta_result2);
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
