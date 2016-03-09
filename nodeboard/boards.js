// boards.js - status board information
const fs = require('fs');

var boardsdata = [
    {
        numval: 0x00,
        source: "whatever",
        labels: [
            "First Aid Kits",
            "Laser Cutter",
            "Table Saw",
            "Bandsaw 1",
            "Fridge",
            "Knife Switch",
            "Server 1",
            "Belt Sander",
        ],
    },
    {
        numval: 0x00,
        source: "whatever",
        labels: [
            "Not Set",
            "Not Set",
            "Not Set",
            "Not Set",
            "Not Set",
            "Not Set",
            "Not Set",
            "Not Set",
        ],
    },
];

function getBoardVal(idx) {
    if(idx > 0 && idx < boardsdata.length){
        return boardsdata[idx].value;
    } else {
        return 666;
    }
}

function setBoardVal(idx, newval) {
    if(idx > 0 && idx < boardsdata.length){
        boardsdata[idx] = newval;
    }
}

function load() {
    try {
        var s = fs.readFileSync("boards.json");
        var d = JSON.parse(s);
        console.log("data is: - ");
        console.log(d);
        if(Array.isArray(d)){
            boardsdata = d; // TODO proper checking!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        }
    } catch (e) {
        console.log(e.message);
        console.log("Resort to using default boards!");
        save();
    }
}

function save() {
    try {
        console.log("saving boards to disk");
        var d = JSON.stringify(boardsdata);
        fs.writeFileSync("boards.json", d);
    } catch (e) {
        console.log(e.message);
        console.log("failed to save boards!");
    }
}

function getval(idx){
    
}

exports.getBoardVal = getBoardVal;
exports.setBoardVal = setBoardVal;
exports.load = load;
exports.save = save;

