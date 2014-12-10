if (!window.console || !console.log)
    console = {log: function () {}}; // all your logs are belong to /dev/null

function handleEvents(data) {
    var htmlItems = [];
    for (var i = 0, l = data.length; i < l; ++i) {
        handleEvent(data[i], htmlItems);
        if (htmlItems.length >= 5) break;
    }
    var eventhtml = htmlItems.length ? htmlItems.join('\n') : '';
    var activity = document.getElementById('githubactivity');
    activity.innerHTML = eventhtml;
}

// Check if two authors are equal.
function author_equal(a, b) {
    return a['name'] == b['name'] && a['email'] == b['email'];
}

// If all authors in the array of commits are equal, return the author.
// Otherwise, return null.
function same_author(commits) {
    if (commits.length == 0) return null;
    for (var i = 1, l = commits.length; i < l; ++i) {
        if (!author_equal(commits[i-1]['author'], commits[i]['author'])) return null;
    }
    return commits[0]['author'];
}

// If all authors in a push are the same, use the author's Git realname.
// Otherwise, use the GitHub user name.
function abbreviate_push(ev) {
    var pl = ev['payload'];
    var author = same_author(pl['commits']);
    if (author == null)
        return handleanyevent(ev, 'pushed to '+printref(pl['ref']));
    ev['actor']['realname'] = author['name'];
    return handleanyevent(ev, 'committed to '+printref(pl['ref']));
}

function handleEvent(ev, html) {
    var pl = ev['payload'];
    switch (ev['type']) {
        case "PushEvent":
            html.push(abbreviate_push(ev));
            var commits = pl['commits'];
            for (var i = 0, l = commits.length; i < l; ++i) {
                if (i > 2 && l > 4) {
                    html.push("and "+(l-i)+" more commits");
                    break;
                }
                var commit = commits[i];
                html.push(['<li class="commit"><a href="https://github.com/jasn/RASMUS/commit/',commit.sha,'">',
                          commit.sha.substring(0,7),'<\/a> ',
                          commit.message.substring(0,70).replace(/&/g,'&amp;').replace(/</g,'&lt;'),
                          '</li>'].join(''));
            }
            break;
        case "CreateEvent":
        case "DeleteEvent":
            var name = pl['ref'];
            if (pl['ref_type'] == 'branch') name = branchlink(name);
            var verbs = {
                'CreateEvent': 'created',
                'DeleteEvent': 'deleted'
            };
            html.push(handleanyevent(ev, verbs[ev['type']]+' '+pl['ref_type']+' '+name));
            break;
        case "IssueCommentEvent":
            var pl = ev['payload'];
            var issuedata = pl['issue'];
            var url = issuedata['html_url'];
            var issue_number = issuedata['number'];
            var desc = 'commented on <a href="'+url+'">issue '+issue_number+'</a>';
            html.push(handleanyevent(ev, desc));
            break;
        default:
            break;
    }
}

// Calculate the user's local time zone
function calc_tz() {
    // In Firefox, Date.toString() returns something like:
    // Thu May 31 2012 11:37:42 GMT+0200 (CEST)
    var s = ""+(new Date);
    var re = new RegExp("\\(([A-Z]{3,4})\\)", "g");
    var o = re.exec(s);
    if (o)
        return o[1];
    else
        return '';
}

var tz = calc_tz();

// ISO 8601 specifies YYYY-MM-DDTHH:MM:SSZ
function printdate(iso8601) {
    var d = new Date(iso8601);
    var now = new Date;
    var a = '<span title="'+d+'">', b = '</span>';
    if (d.getFullYear() != now.getFullYear() || d.getMonth() != now.getMonth() || d.getDate() != now.getDate()) {
        return a+'on '+d.toLocaleDateString()+b;
    }
    return a+'on '+d.toLocaleTimeString()+' '+tz+b;
}

function branchlink(branch) {
    return '<a href="https://github.com/jasn/RASMUS/tree/'+branch+'">'+branch+'</a>';
}

function printref(refname) {
    if (refname.substring(0, 11) == 'refs/heads/') {
        var branch = refname.substring(11, refname.length);
        return branchlink(branch);
    }
    return refname;
}

function handleanyevent(ev, desc) {
    var actor = ev['actor'];
    return [
        '<li><a href="https://github.com/',actor['login'],'">',(actor['realname'] || actor['login']),'</a> ',
        desc,
        ' <span class="date">',printdate(ev['created_at']),'</span></li>',
    ''].join('');
}

if ( typeof String.prototype.endsWith != 'function' ) {
    String.prototype.endsWith = function( str ) {
	return this.substring( this.length - str.length, this.length ) === str;
    }
};

function handleReleases(data) {
    var rr=[];
    for (var i = 0, l = data.length; i < l; ++i) {
	var rel=data[i];
	if (rel['draft']) continue;
	
	rr=rr.concat(['<div class="release"><span class="title">RASMUS ', rel['name'], '</span>',
	       '<span class="date"> Released on ', printdate(rel['published_at']), '</span>',
		      '<span class="body">', rel['body'], '</span>',
		      '<ul class="downloads">'
		     ]);
	var assets=rel['assets'];
	console.log(assets);
	for (var j = 0; j < assets.length; ++j) {
	    var asset=assets[j];
	    var c='';
	    var name=asset['name'];
	    if (name.endsWith('-win.zip')) {
		name='Window 64bit';
		c=' windows';
	    } else if (name.endsWith('-osx.zip') || name.endsWith('.dmg')) {
		name='OSX 64bit';
		c=' osx';
	    }
	    rr=rr.concat([
		'<li class="downloaditem', c , '"><a href="', asset['browser_download_url'],
		'">', name, '</a></li>']);
	}
	
	rr=rr.concat([
	    '<li class="downloaditem sourcetar"><a href="',
	    'https://github.com/jasn/RASMUS/archive/', rel['tag_name'], '.tar.gz',
	    '">Source code (tar.gz)</a></li>',
	    '<li class="downloaditem sourcetar"><a href="',
	    'https://github.com/jasn/RASMUS/archive/', rel['tag_name'], '.zip',
	    '">Source code (zip)</a></li>',
	    '</ul></div>'
	]);
    }
    document.getElementById('githubreleases').innerHTML = rr.join('');
}

function handleCached(url, callback) {
    if (!localStorage) return false;
    var str = localStorage.getItem(url);
    if (!str) return false;
    var data = JSON.parse(str);
    if (!data) return false;
    if (!data['input'] || !data['cachetime']) return false;
    var age = new Date().getTime() - data['cachetime'];
    if (age < 0 || age > 30000) return false;
    callback(data['input']);
    return true;
}

function handleRemote(url, callback) {
    if (handleCached(url, callback)) return;
    $.getJSON(url, function(input) {
	if (localStorage) 
	    localStorage.setItem(url, JSON.stringify(
		{'input': input, 'cachetime': new Date().getTime()}));
	callback(input);
    });
}

$(document).ready(function() {
    handleRemote("https://api.github.com/repos/jasn/RASMUS/events", handleEvents);
    handleRemote("https://api.github.com/repos/jasn/RASMUS/releases", handleReleases);
});

// vim:set sw=4 sts=4 et:
