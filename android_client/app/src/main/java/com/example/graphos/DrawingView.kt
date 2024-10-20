package com.example.graphos

import android.content.Context
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Path
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View

class DrawingView(context: Context, attrs: AttributeSet?) : View(context, attrs) {

    private var path: Path = Path()
    private var paint: Paint = Paint()

    interface DrawingChangeListener {
        fun onDrawingChanged()
    }

    private var listener: DrawingChangeListener? = null

    fun setDrawingChangeListener(listener: DrawingChangeListener) {
        this.listener = listener
    }

    init {
        // Initialize the paint with desired attributes
        paint.apply {
            color = Color.WHITE // drawing color
            isAntiAlias = true
            strokeWidth = 10f // stroke width
            style = Paint.Style.STROKE // Set style to stroke (not fill)
            strokeJoin = Paint.Join.ROUND // Round joins for paths
            strokeCap = Paint.Cap.ROUND // Rounded ends for strokes
        }
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        // Draw the path onto the canvas
        canvas.drawPath(path, paint)
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        val x = event.x
        val y = event.y

        when (event.action) {
            MotionEvent.ACTION_DOWN -> {
                // Start the path at the user's touch point
                path.moveTo(x, y)
                return true
            }
            MotionEvent.ACTION_MOVE -> {
                // Draw the path as the user moves their finger
                path.lineTo(x, y)
            }
            MotionEvent.ACTION_UP -> {
                // Do nothing on action up
            }
            else -> return false
        }
        // Invalidate the view to trigger a redraw
        invalidate()


        return true
    }

    // Function to clear the drawing
    fun clear() {
        path.reset()
        invalidate()
    }
    fun getBitmap(): Bitmap {
        // Create a bitmap with the same size as the view
        val originalBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888)
        val canvas = Canvas(originalBitmap)
        // Draw the view onto the canvas
        draw(canvas)

        return originalBitmap
    }

}
