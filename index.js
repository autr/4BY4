import serialport from 'serialport'
import readline from '@serialport/parser-readline'
import { Client } from 'node-osc'
let _PORT


let OSC = {}


async function create( localAddress, localPort ) {

    return new Promise( (resolve,reject) => {
        if (!localAddress) localAddress = "0.0.0.0"
        if (!localPort) localPort = 4444
        let key = localAddress + ':' + localPort
        if (!OSC[key]) {
            OSC[key] = new Client(localAddress,localPort)
            resolve( OSC[key] )
        } else {
            console.log(`[lockd] ${key} already exists!`)
            reject('already exists')
        }
    })

}

const run = async e => {
    let list = await serialport.list()
    let arduino = list.find( t => (t?.manufacturer||'').toLowerCase().indexOf( 'arduino' ) != -1 )

    await create()

    if (!arduino) throw 'no arduino found'
    _PORT = new serialport(arduino.path)

    _PORT.on('open', onOpen)
    _PORT.on('close', onClose)
    _PORT.pipe(new readline({ delimiter: '\r\n' })).on('data', onData)

}

async function onOpen(e) {

    console.log(`[lockd] ✅  opened`)

}
async function onClose(e) {

    console.log(`[lockd] 🛑  closed`)

}
async function onData(data) {


    let bits = data.split(':')
    if (bits[0] == 'change') {
        let num = parseInt(bits[1])
        let val = parseInt(bits[2])

        let idx = num%4
        let six = num%16
        let page = (num-(six))/16
        let letter = ['A','B','C','D'][(six-idx)/4]

        idx += 1
        page += 1

        let value = 1 - (val/1024)
        let address = `/P${page}/${letter}/${idx}`

        console.log(`[lockd] sending: ${address} ${value.toFixed(2)}`)

        for (const [key,o] of Object.entries(OSC) ) {
            o.send( address, value )
        }
    } else {

        console.log(`[lockd] received: ${data}`)
    }
}

run()