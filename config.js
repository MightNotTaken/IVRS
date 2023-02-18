const fs = require('fs')
for (let i=1; i<4; i++) {
    fs.copyFileSync('1.wav', `files/${i}.wav`)
    for (let j=1; j<4; j++) {
        fs.copyFileSync('1.wav', `files/${i}${j}.wav`)
        for (let k=1; k<4; k++) {
            fs.copyFileSync('1.wav', `files/${i}${j}${k}.wav`)
            for (let l=1; l<4; l++) {
                fs.copyFileSync('1.wav', `files/${i}${j}${k}${l}.wav`)
            }
        }
    }
}
