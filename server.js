const express = require('express');
const bodyParser = require('body-parser');
const path = require('path');

const app = express();
const port = 3000;

let lastDataTime = new Date();
let sensorData = {};

app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, 'public')));

// Route to serve the index.html file
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

app.post('/api/data', (req, res) => {
    sensorData = req.body;
    lastDataTime = new Date();
    res.send('Data received');
});

app.get('/api/data', (req, res) => {
    res.json({ sensorData, lastDataTime });
});

app.listen(port, () => {
    console.log(`Server running at http://localhost:${port}`);
});
