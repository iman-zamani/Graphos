package com.example.graphos


import android.graphics.Bitmap
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import android.widget.TextView
import kotlinx.coroutines.*
import java.io.OutputStream
import java.net.*
import com.example.graphos.databinding.ActivityMainBinding
import java.security.MessageDigest
import android.util.Base64
import java.io.ByteArrayOutputStream

class MainActivity : AppCompatActivity(), DrawingView.DrawingChangeListener {
    private lateinit var coordinatesTextView: TextView
    private val coroutineScope = CoroutineScope(Dispatchers.IO)
    private var serverIp: String? = null
    private val listenPort = 5555
    private val expectedResponse = "Hello from Graphos Desktop app"
    private val message = "Hello from Graphos android app"
    private var paired = false

    private lateinit var binding: ActivityMainBinding
    private var previousBitmapHash: String? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        // Initialize View Binding
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        coordinatesTextView = binding.coordinatesTextView
        coordinatesTextView.text = "Pairing..."

        // Set the DrawingChangeListener
        binding.drawingView.setDrawingChangeListener(this)

        // Clear button action
        binding.clearButton.setOnClickListener {
            binding.drawingView.clear()
        }

        // Start server discovery
        discoverServer()

        startPeriodicSending()
    }

    private fun startPeriodicSending() {
        coroutineScope.launch {
            while (isActive) {
                if (paired) {
                    sendDrawing()
                }
                delay(100)  // Wait 100 milliseconds
            }
        }
    }
    private fun discoverServer() {
        coroutineScope.launch {
            while(!paired){
                try {
                    // Listen for server broadcast
                    DatagramSocket(listenPort).use { socket ->
                        val receiveData = ByteArray(1024)
                        val receivePacket = DatagramPacket(receiveData, receiveData.size)
                        socket.receive(receivePacket)
                        val response = String(receivePacket.data, 0, receivePacket.length).trim()

                        if (response == expectedResponse) {
                            serverIp = receivePacket.address.hostAddress
                            updateUI("Paired successfully")
                            paired = true
                        }
                    }
                } catch (e: Exception) {
                    updateUI("Failed to pair")
                    e.printStackTrace()
                }
                serverIp?.let { ip ->
                    sendToServer(message, ip)
                }
            }}
    }

    private fun updateUI(text: String) {
        runOnUiThread {
            coordinatesTextView.text = text
        }
    }

    override fun onDrawingChanged() {
        if (!paired) {
            return
        }
        coroutineScope.launch {
            val bitmap = binding.drawingView.getBitmap()
            val bitmapHash = hashBitmap(bitmap)
            if (bitmapHash != previousBitmapHash) {
                previousBitmapHash = bitmapHash
                val bitmapBytes = bitmapToByteArray(bitmap)
                serverIp?.let { ip ->
                    sendBitmapToServer(bitmapBytes, ip)
                }
            }
        }
    }
    private fun rotateBitmap(source: Bitmap): Bitmap {
        val matrix = android.graphics.Matrix()
        matrix.postRotate(90f) // Rotate the bitmap by 90 degrees
        return Bitmap.createBitmap(source, 0, 0, source.width, source.height, matrix, true)
    }

    private fun sendDrawing() {
        if (!paired) {
            return
        }
        coroutineScope.launch {
            val bitmap = binding.drawingView.getBitmap()

            // Check if the bitmap is empty or blank
            if (isBitmapEmpty(bitmap)) {
                val emptyByteArray = ByteArray(0)
                serverIp?.let { ip ->
                    sendBitmapToServer(emptyByteArray, ip)
                }
                return@launch
            }

            val rotatedBitmap = rotateBitmap(bitmap)
            val bitmapBytes = bitmapToByteArray(rotatedBitmap)
            serverIp?.let { ip ->
                sendBitmapToServer(bitmapBytes, ip)
            }
        }
    }


    private fun hashBitmap(bitmap: Bitmap): String {
        val stream = ByteArrayOutputStream()
        bitmap.compress(Bitmap.CompressFormat.PNG, 100, stream)
        val bytes = stream.toByteArray()
        val md = MessageDigest.getInstance("MD5")
        val digest = md.digest(bytes)
        return Base64.encodeToString(digest, Base64.DEFAULT)
    }

    private fun sendBitmapToServer(bitmapBytes: ByteArray, ip: String) {
        try {
            Socket().use { socket ->
                socket.connect(InetSocketAddress(ip, 5000), 5000)
                val outputStream: OutputStream = socket.getOutputStream()

                // Send the length of the bitmap first
                val lengthBytes = bitmapBytes.size.toString().toByteArray()
                outputStream.write(lengthBytes)
                outputStream.write("\n".toByteArray()) // Add a newline as a separator

                // Send the actual PNG image data
                outputStream.write(bitmapBytes)

                // Flush and close
                outputStream.flush()
            }
        } catch (e: Exception) {
            updateUI("Error: ${e.message}")
            e.printStackTrace()
        }
    }

    private fun bitmapToByteArray(bitmap: Bitmap): ByteArray {
        val stream = ByteArrayOutputStream()
        bitmap.compress(Bitmap.CompressFormat.PNG, 100, stream) // Compress bitmap as PNG
        return stream.toByteArray()
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
    private fun isBitmapEmpty(bitmap: Bitmap): Boolean {

        if (bitmap.width == 0 || bitmap.height == 0) {
            return true
        }

        val pixels = IntArray(bitmap.width * bitmap.height)
        bitmap.getPixels(pixels, 0, bitmap.width, 0, 0, bitmap.width, bitmap.height)

        // Check if all pixels are transparent
        for (pixel in pixels) {
            val alpha = pixel shr 24 and 0xff // Get the alpha channel
            if (alpha != 0) {
                return false
            }
        }

        return true // all pixels are transparent
    }
}

