var server = "http://" + window.location.hostname + ":" + window.location.port;

function command() {
  var query = document.getElementById("query").value.toLowerCase();
  var query = query.replace(/ /g, "+");
  $.ajax({url: server + "/",
    data: "qf=" + query,
    dataType: "json",
    success: callback});
  query.value = "";
}

function callback(data, status, xhr) {
  var view_left = "";
  for (var i in data.results.items) {
    var title = data.results.items[i]["htmlTitle"];
    var snippet = data.results.items[i]["snippet"];
    var link = data.results.items[i]["link"];
    var link_name = data.results.items[i]["displayLink"]
    var element = "<p>" + "<h2>" + title  + "</h2>"
        + snippet + "<br />"
        + "<a href=\"" + link + "\">" + link_name + "</a></p>";
    view_left += element;
    // $.cookie("view-left-title" + i, title);
    // $.cookie("view-left-snippet" + i, snippet);
    // $.cookie("view-left-link" + i, link);
    // $.cookie("view-left-link-name" + i, link_name);
  }
  // $.cookie("view-left" + data.results.items.length, null);
  if (data.results.items.length == 0) {
    view_left += "<h2>no results</h2>";
  }
  $("#view-left").html(view_left);

  var meta_result1 = "<div id=\"meta-result-area1\">";
  for (var i in data.target_keywords) {
    var keyword = data.target_keywords[i];
    var element = "<h2>" + keyword + "</h2>";
    if (i > 0) {
      meta_result1 += ", ";
    }
    meta_result1 += element;
  }
  meta_result1 += " <span style=\"font-size: 0.61em;\">/</span> " +
                  data.target_type + "</div>";
  // $.cookie("meta-result-target", meta_result1);
  $("#meta-result-area1").replaceWith(meta_result1);

  var meta_result2 = "<div id=\"meta-result-area2\">";
  for (var i in data.entities) {
    var name = data.entities[i][0];
    var rank = data.entities[i][1];
    var element = "<h2>" + name + "</h2>:" + rank + "   ";
    meta_result2 += element;
    // $.cookie("meta-result-name" + i, name);
    // $.cookie("meta-result-rank" + i, rank);
  }
  // $.cookie("meta-result" + data.entities.length, null);
  if (data.entities.length == 0) {
    meta_result2 += "no entities found";
  }
  meta_result2 += "</div>";
  $("#meta-result-area2").replaceWith(meta_result2);
}

$(document).ready (
  function() {
    var i = 0;
    var title = $.cookie("view-left-title0");
    var view_left = "<div id=\"view-left\">";
    while (title) {
      var snippet = $.cookie("view-left-snippet" + i);
      var link = $.cookie("view-left-link" + i);
      var link_name = $.cookie("view-left-link-name" + i);
      var element = "<p>" + "<h2>" + title  + "</h2>"
          + snippet + "<br />"
          + "<a href=\"" + link + "\">" + link_name + "</a></p>";
      view_left += element;
      title = $.cookie("view-left-title" + ++i);
    }
    view_left += "</div>";
    if (i > 0) {
      $("#view-left").replaceWith(view_left);
    }

    i = 0;
    var name = $.cookie("meta-result-name0");
    var meta_result = "<div id=\"meta-result-area2\">";
    while (name) {
      var rank = $.cookie("meta-result-rank" + i);
      var element = "<h2>" + name + "</h2>:" + rank + "   ";
      meta_result += element;
      name = $.cookie("meta-result-name" + ++i);
    }
    meta_result += "</div>";
    if (i > 0) {
      $("#meta-result-area1").replaceWith($.cookie("meta-result-target"));
      $("#meta-result-area2").replaceWith(meta_result);
    }
  }
);

$(document).keypress (
  function(event) {
    if (event.which == 13) {
      event.preventDefault();
      command();
    }
  }
);
