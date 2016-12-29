      const net = require("net");
      const mp = require("msgpack5")();
      const s = new require('stream').PassThrough({objectMode:true});
      const bl = new require('bl');
      const fs = require("fs");
      const en = require("../").encode();
      const dc = require("../").decode();
      const stream = require("stream");

      var obuffer = new stream.PassThrough();


// Initiate the source

      const error = function(msg){
	var _msg = (msg) ? msg : "Usage: " + __filename + " <use case.js>";
	console.error(_msg);
    	process.exit(-1);
      };

     if (process.argv.length <= 2) {
	error();
     } 
     	en.on('data',(d)=>{
		console.log(d);
		msg = d;
	});


     
     var msg; 
      try {	 
      	var contents = fs.readFileSync(process.argv[2]);
	var e = eval(contents.toString());
	console.log(e);
	console.log(mp.decode(mp.encode(e)));
        en.end(mp.encode(e));
	
	} catch (e) { 
		error(e);
	}

	var buffer=Buffer(0);

	
      const socketPath = "/tmp/test.sock";
      const c = net.createConnection(socketPath);
      var conn;
      c.setTimeout(100);
     /* c.on('timeout', () => {
	if (buffer.length >5){
	 try{
	    process.stdout.write(mp.decode(buffer).toString() + "\n");
	 } catch(e){
		console.log(buffer);
		console.log(buffer.length);
		console.error(e);
	} finally{
	    buffer=[]	  
	  }	
	}
	
	});*/
      c.on('error', (e) => {
	console.log("error " + e.toString());
	process.exit(0);
        conn = null;
        return;
      });
      c.on('connect', () => {
        conn = c;
        console.log("connected");
	setInterval(()=>{
	//console.log("sending");
	c.write(msg);
	},5000);
      });
      c.pipe(dc,{end:false});
      dc.on('data',(data)=>{
	buffer = Buffer.concat([buffer,data]);
      }); 	
      dc.on("flush", ()=>{
	console.log(mp.decode(buffer));
	buffer =Buffer(0);
      });












