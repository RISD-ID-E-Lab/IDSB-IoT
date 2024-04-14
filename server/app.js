const express = require('express')
const bodyParser = require('body-parser')
const app = express()
const port = process.env.PORT || 4000;

app.use(bodyParser.json())

app.get('/', (req, res) => {
  res.send('Hello World!')
})

let reset = false;

app.post('/', (req, res) => {
    console.log('request: POST ' + JSON.stringify(req.body))
    res.send(reset);
})

app.get('/reset', (req, res) => {
    reset = !reset;
    console.log("reset: " + reset)
    res.send(reset)
})

app.listen(port, () => {
  console.log(`Example app listening on port ${port}`)
})