package com.example.graphos

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import java.io.OutputStream
import java.net.InetSocketAddress
import java.net.Socket
import android.view.MotionEvent
import android.widget.TextView


import kotlinx.coroutines.*

class MainActivity : AppCompatActivity() {
    private lateinit var coordinatesTextView: TextView
    private val coroutineScope = CoroutineScope(Dispatchers.IO)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        coordinatesTextView = findViewById(R.id.coordinatesTextView)
        coordinatesTextView.text = "NULL"
    }

    override fun onTouchEvent(event: MotionEvent?): Boolean {
        event?.let {
            when (it.action) {
                MotionEvent.ACTION_DOWN, MotionEvent.ACTION_MOVE -> {
                    val x = it.x
                    val y = it.y
                    coordinatesTextView.text = "X: $x, Y: $y"
                    coroutineScope.launch {
                        sendToServer("X: $x, Y: $y")
                    }
                }
                MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                    coordinatesTextView.text = "NULL"
                    coroutineScope.launch {
                        sendToServer("NULL")
                    }
                }

                else -> {}
            }
        }
        return super.onTouchEvent(event)
    }

    private fun sendToServer(value: String) {
        try {
            // temporary
            val serverIp = "192.168.112.117"
            val serverPort = 5000
            val socket = Socket()

            socket.connect(InetSocketAddress(serverIp, serverPort), 5000)
            val outputStream: OutputStream = socket.getOutputStream()
            outputStream.write(value.toByteArray())
            outputStream.flush()
            socket.close()

        } catch (e: Exception) {
            e.printStackTrace()
        }
    }
}

