/**
 * Created by elvischen on 07/05/2017.
 */

var http = require("http"),
    fs = require("fs");

http.createServer(function(req, res) {
    console.log("Request received");
    if (req.url == '/upload' && req.method.toLowerCase() == 'post') {
        // parse a file upload
        res.setHeader("Access-Control-Allow-Origin","*");
        res.setHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");

        var writeStream = fs.createWriteStream('/home/ubuntu/output.mp4');
        req.pipe(writeStream);
        // This pipes the POST data to the file
        var cmdcd = "cd /home/ubuntu/senseface/samples/c++";
        var cppname = "/home/ubuntu/senseface/samples/c++/test_sample_face_track";
        var pyname = "python /home/ubuntu/pack/eye_effect.py";
        var source = " /home/ubuntu/output.mp4";
        var dest = " /home/ubuntu/1.avi";
        var txt = " /home/ubuntu/1.txt";
        var cmdStr1 = cppname + source + txt;
        var cmdStr2 = pyname + source + dest + txt + " 1";

        var child_process = require('child_process');
        console.log(cmdcd + " && " + cmdStr1 + " && " + cmdStr2);
        child_process.exec(cmdcd + " && " + cmdStr1 + " && " + cmdStr2,
            function (error, stdout, stderr) {
                if (error) {
                    console.log(error);
                } else {
                    console.log(stdout);
                }});

        // After all the data is saved, respond with a simple html form so they can post more data
        req.on('end', function () {
            res.writeHead(200, {"content-type":"text/html"});
            res.write('received upload:\n\n');
            res.end('<form method="POST"><input name="test" /><input type="submit"></form>');
        });

        // This is here incase any errors occur
        writeStream.on('error', function (err) {
            console.log(err);
        });
        return;
    }

    // show a file upload form
    res.setHeader("Access-Control-Allow-Origin","*");
    res.setHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    res.writeHead(200, {'content-type': 'text/html'});
    res.end(
        '<form action="/upload" enctype="multipart/form-data" '+
        'method="post">'+
        '<input type="text" name="title"><br>'+
        '<input type="file" name="upload" multiple="multiple"><br>'+
        '<input type="submit" value="Upload">'+
        '</form>'
    );
}).listen(8888);
console.log('server started');
