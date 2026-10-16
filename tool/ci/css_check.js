#!/usr/bin/env node
// css_check.js FILE... : parse each stylesheet with postcss; print "file:line:col: message" for
// every CssSyntaxError and exit 1 if any file failed. postcss must be resolvable (NODE_PATH).
'use strict';
const fs = require('fs');
let postcss;
try {
	postcss = require('postcss');
} catch (e) {
	console.error('css_check: postcss is not installed (set NODE_PATH to a node_modules containing it)');
	process.exit(2);
}
let failed = 0;
for (const file of process.argv.slice(2)) {
	try {
		postcss.parse(fs.readFileSync(file, 'utf8'), { from: file });
	} catch (e) {
		failed++;
		if (e.name === 'CssSyntaxError')
			console.log(`${file}:${e.line}:${e.column}: ${e.reason}`);
		else
			console.log(`${file}: ${e.message}`);
	}
}
process.exit(failed ? 1 : 0);
