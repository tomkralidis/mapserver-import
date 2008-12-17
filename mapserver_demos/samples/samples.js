
var samples = [
               {title: 'Hello World',
                url: 'hello_world/index.html',
                description: 'Places the text \'Hello World!\" in an image.',
                keywords: ['inline features', 'annotation']},
               {title: 'Relative Positioning',
                url: 'relative_positioning/index.html',
                description: 'Positioning of features relative to places in an image (e.g. lower left corner).',
                keywords: ['inline features', 'relative position']},
	       {title: 'Simple Lines',
                url: 'simple_lines/index.html',
                description: 'Drawing of simple linear features.',
                keywords: ['inline features', 'line layers']},
];

function launchSample(i) {
  var url = samples[i].url;

  var sep = '?';
  for (var key in samples[i].args) {
    url += sep + key + '=' + escape(samples[i].args[key]);
    sep = '&';
  }

  parent.samples_window.location.href = url;
}

function displaySamples(name, filter) {
  var element = document.getElementById(name);
  element.innerHTML = '';

  for(var i=0; i<samples.length; i++) {
    if(filter) {

    }

    var content = "<div class=\"sample_container\">";
    content += "<a href=\"javascript:launchSample(" + i + ")\">";
    content += "<span class=\"sample_title\">" + samples[i].title + "</span><br>";
    content += "<span class=\"sample_description\">" + samples[i].description + "</span>";

    element.innerHTML += content;
  }

  // element.innerHTML += '</samples>';
}
