package com.example.graphos

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import android.view.MotionEvent
import android.widget.TextView
import kotlinx.coroutines.*
import java.io.OutputStream
import java.net.*

class MainActivity : AppCompatActivity() {
    private lateinit var coordinatesTextView: TextView
    private val coroutineScope = CoroutineScope(Dispatchers.IO)
    private var serverIp: String? = null
    private val listenPort = 5555
    private val expectedResponse = "Hello from Graphos Desktop app"
    private val message = "Hello from Graphos android app"
    private var paired = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        coordinatesTextView = findViewById(R.id.coordinatesTextView)
        coordinatesTextView.text = "pairing..."

        discoverServer()
    }

    private fun discoverServer() {
        coroutineScope.launch {
            try {
                // Listen for server broadcast
                DatagramSocket(listenPort).use { socket ->
                    //socket.soTimeout = 10000  // Set a timeout for the listen duration
                    val receiveData = ByteArray(1024)
                    val receivePacket = DatagramPacket(receiveData, receiveData.size)
                    socket.receive(receivePacket)
                    val response = String(receivePacket.data, 0, receivePacket.length).trim()

                    if (response == expectedResponse) {
                        serverIp = receivePacket.address.hostAddress
                        updateUI("paired successfully")
                        paired = true
                    }
                }
            } catch (e: Exception) {
                updateUI("Failed")
                e.printStackTrace()
            }
            serverIp?.let { ip ->
                sendToServer(message, ip)
            }
        }
    }

    private fun updateUI(text: String) {
        runOnUiThread {
            coordinatesTextView.text = text
        }
    }

    override fun onTouchEvent(event: MotionEvent?): Boolean {
        if (!paired){
            return false;
        }
        event?.let {
            when (it.action) {
                MotionEvent.ACTION_DOWN, MotionEvent.ACTION_MOVE -> {
                    val x = it.x
                    val y = it.y
                    coordinatesTextView.text = "X: $x, Y: $y"
                    coroutineScope.launch {
                        serverIp?.let { ip ->
                            sendToServer("X: $x, Y: $y", ip)
                        }
                    }
                }
                MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                    coordinatesTextView.text = "NULL"
                    coroutineScope.launch {
                        serverIp?.let { ip ->
                            sendToServer("NULL", ip)
                        }
                    }
                }
                else -> println("Error in onTouchEvent function")
            }
        }
        return super.onTouchEvent(event)
    }

    private fun sendToServer(value: String, ip: String) {
        try {
            Socket().use { socket ->
                socket.connect(InetSocketAddress(ip, 5000), 5000)
                val outputStream: OutputStream = socket.getOutputStream()
                outputStream.write(value.toByteArray())
                outputStream.flush()
            }
        } catch (e: Exception) {
            updateUI("Error: ${e.message}")
            e.printStackTrace()
        }
    }
}
