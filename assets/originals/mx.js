function mx5(){$.get("run",function(t){}),mx5d()}function mx5s(){$.get("stat",function(t){$("#stat").html(t),setTimeout(function(){mx5s()},500)})}function mx5d(){$.get("dtc",function(t){$("#dtc").html(t),setTimeout(function(){mx5d()},5e3)})}