#ifndef PAGES_H
#define PAGES_H

const char PAGE_INDEX[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<title>CYD Dashboard</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; background: #f4f4f9; color: #333; display: flex; flex-direction: column; align-items: center; }
.card { background: white; padding: 2rem; border-radius: 12px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); width: 90%; max-width: 400px; margin-top: 2rem; }
h1 { color: #2c3e50; font-size: 1.5rem; margin-bottom: 1.5rem; }
.btn { display: block; width: 100%; padding: 0.8rem; margin: 0.5rem 0; text-align: center; background: #3498db; color: white; text-decoration: none; border-radius: 6px; transition: background 0.2s; box-sizing: border-box; }
.btn:hover { background: #2980b9; }
.btn-alt { background: #95a5a6; }
.btn-alt:hover { background: #7f8c8d; }
.status { font-size: 0.9rem; color: #27ae60; margin-top: 1rem; }
</style>
</head>
<body>
<div class="card">
    <h1>CYD Dashboard</h1>
    <a href="/note" class="btn">📝 Edit Note</a>
    <a href="/timetable" class="btn">📅 Edit Timetable</a>
    <a href="/files" class="btn btn-alt">📂 SD Card Files</a>
    <p class="status">● System Online</p>
</div>
</body></html>
)rawliteral";

const char PAGE_NOTE[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<title>Edit Note</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; background: #f4f4f9; color: #333; display: flex; flex-direction: column; align-items: center; }
.card { background: white; padding: 2rem; border-radius: 12px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); width: 90%; max-width: 500px; margin-top: 2rem; }
h1 { color: #2c3e50; font-size: 1.5rem; margin-bottom: 1.2rem; }
textarea { width: 100%; padding: 0.8rem; border-radius: 6px; border: 1px solid #ddd; font-size: 1rem; box-sizing: border-box; margin-bottom: 1rem; resize: vertical; }
input[type="submit"] { width: 100%; padding: 0.8rem; background: #2ecc71; color: white; border: none; border-radius: 6px; font-size: 1rem; cursor: pointer; }
input[type="submit"]:hover { background: #27ae60; }
.back { display: inline-block; margin-top: 1rem; color: #7f8c8d; text-decoration: none; font-size: 0.9rem; }
</style>
</head>
<body>
<div class="card">
    <h1>Edit Display Note</h1>
    <form action="/note" method="POST">
        <textarea name="note" rows="5" placeholder="Enter note here...">%NOTE%</textarea>
        <input type="submit" value="Save Note">
    </form>
    <a href="/" class="back">← Back to Dashboard</a>
</div>
</body></html>
)rawliteral";

const char PAGE_TIMETABLE[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<title>Edit Timetable</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; background: #f4f4f9; color: #333; display: flex; flex-direction: column; align-items: center; }
.card { background: white; padding: 2rem; border-radius: 12px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); width: 95%; max-width: 600px; margin-top: 2rem; }
h1 { color: #2c3e50; font-size: 1.5rem; margin-bottom: 1.2rem; }
textarea { width: 100%; padding: 0.8rem; border-radius: 6px; border: 1px solid #ddd; font-family: monospace; font-size: 0.9rem; box-sizing: border-box; margin-bottom: 1rem; resize: vertical; }
input[type="submit"] { width: 100%; padding: 0.8rem; background: #3498db; color: white; border: none; border-radius: 6px; font-size: 1rem; cursor: pointer; }
input[type="submit"]:hover { background: #2980b9; }
.help { font-size: 0.8rem; color: #7f8c8d; margin-bottom: 1rem; line-height: 1.4; }
.back { display: inline-block; margin-top: 1rem; color: #7f8c8d; text-decoration: none; font-size: 0.9rem; }
</style>
</head>
<body>
<div class="card">
    <h1>Edit Timetable (JSON)</h1>
    <p class="help">Format: {"table": [["Subject", "Subject", ...], ["Next Row", ...]]}<br>Each inner array is a row (usually 6 columns).</p>
    <form action="/timetable" method="POST">
        <textarea name="timetable" rows="12">%TIMETABLE%</textarea>
        <input type="submit" value="Update Timetable">
    </form>
    <a href="/" class="back">← Back to Dashboard</a>
</div>
</body></html>
)rawliteral";

#endif // PAGES_H