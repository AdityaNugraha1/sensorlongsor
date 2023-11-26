// app.js
const express = require('express');
const mysql = require('mysql');
const axios = require('axios'); 

// MySQL connection
const db = mysql.createConnection({
  host: '34.128.102.195',
  user: 'adit',
  password: 'adit1234',
  database: 'sensorlongsor'
});

db.connect((err) => {
  if (err) throw err;
  console.log('Connected to database');
});

const app = express();

// CORS Middleware (for cross-origin requests)
app.use((req, res, next) => {
  res.header("Access-Control-Allow-Origin", "*");
  res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
  next();
});

// Route to get sensor data
app.get('/api/sensor_data', (req, res) => {
  db.query('SELECT * FROM log', (err, results, fields) => {
    if (err) throw err;
    res.json(results);
  });
});

// Route to insert sensor data
app.get('/api/insert_sensor_data', (req, res) => {
  let soilMoisture = req.query.soilMoisture;
  let raindrops = req.query.raindrops;
  let potentialLandslide = req.query.potentialLandslide;
  let latitude = req.query.latitude;
  let longitude = req.query.longitude;
  let maps = `https://maps.google.com?q=${latitude},${longitude}`

  let query = `INSERT INTO log (soilMoisture, raindrops, potentialLandslide, Maps) VALUES ('${soilMoisture}', '${raindrops}', '${potentialLandslide}', '${maps}')`;
  db.query(query, (err, result) => {
    if (err) {
      res.status(500).send('Failed to insert data');
      throw err;
    }

    // Check if potentialLandslide is 'yes' and send a Telegram message
    if (potentialLandslide.toLowerCase() === 'yes') {
      const botToken = '6882820427:AAFF3q_maz7d3yPdIpvpvmuhFJt_2wAEvDA';
      const chatId = '611793347';
      const message = `Peringatan Longsor\nSoil Moisture : ${soilMoisture}\nRaindrops : ${raindrops}\nMaps : ${maps}`;

      axios.get(`https://api.telegram.org/bot${botToken}/sendMessage`, {
        params: {
          chat_id: chatId,
          text: message
        }
      })
      .then(response => {
        console.log('Message sent to Telegram:', response.data);
      })
      .catch(error => {
        console.error('Error sending message to Telegram:', error);
      });
    }

    res.status(200).send('Data Inserted');
  });
});

const PORT = process.env.PORT || 9000;

app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});
