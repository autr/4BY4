import serialport from 'serialport'
import readline from '@serialport/parser-readline'

let _PORT

const run = async e => {
    let list = await serialport.list()
    let arduino = list.find( t => (t?.manufacturer||'').toLowerCase().indexOf( 'arduino' ) != -1 )

    console.log(list, arduino)

    if (!arduino) throw 'no arduino found'

    _PORT = new serialport(arduino.path)
    _PORT.on('open', onOpen)
    const parser = _PORT.pipe( new readline({ delimiter: '\n' }));
    // _PORT.on('data', onData)
}

async function onOpen(e) {
    console.log(`[Mnidimonic] opened ${e}`, e)
}
async function onData(data) {
    console.log(`[Mnidimonic] received`, data)
}

run()