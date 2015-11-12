#include <stdio.h>
#include <unistd.h>
#include <cv.h>
#include <highgui.h>

int main(int argc, char **argv)
{
	// get command line parameters
	if(argc < 4)
	{
		printf("Usage: %s <video> <vga width> <vga height>\n", argv[0]);
		printf("Usage: %s <camera id> <vga width> <vga height>\n", argv[0]);
		exit(0);
	}
	const char *filename = argv[1];
	int width = atoi(argv[2]);
	int height = atoi(argv[3]);
	int outputPixelCount = width * height;

	// opencv initializations
	CvCapture* cap = cvCaptureFromFile(filename);
	if (!cap)
		cap = cvCaptureFromCAM(atoi(filename));
	if (!cap)
	{
		printf("Could not open file/camera!\n");
		exit(1);
	}
	IplImage* frame = cvQueryFrame(cap); // get first frame for size
	if (!frame)
	{
		printf("The Video is empty!\n");
		cvReleaseCapture(&cap);
		exit(1);
	}
	IplImage* edges = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
	IplImage* lines = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);
	IplImage* out = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	int outStep = out->widthStep;
	int outChannels = out->nChannels;
	unsigned char *outData = (unsigned char*)out->imageData;

	// position windows
	cvNamedWindow("frame", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("frame", 0, 32);
	cvNamedWindow("edges", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("edges", frame->width, 32);
	cvNamedWindow("lines", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("lines", 2 * frame->width, 32);
	cvNamedWindow("out", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("out", -1, 900 - height - 24); // TODO: make VGA window position configurable

	while (42)
	{
		frame = cvQueryFrame(cap);
		if (!frame || (cvWaitKey(1) & 0xff) == 'q')
			break;
		cvShowImage("frame", frame);

		// edge detection
		cvCanny(frame, edges, 128.0, 130.0, 3); // TODO: tweak parameters?
		cvShowImage("edges", edges);

		// get contours
		CvMemStorage *storage = cvCreateMemStorage(0);
		CvSeq *contours;
		int contourCount = cvFindContours(
			edges, storage, &contours, sizeof(CvContour),
			CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
		cvZero(lines);
		cvDrawContours(lines, contours, cvScalar(0, 255, 0, 255), cvScalarAll(0), 100, 1, 8, cvPoint(0,0));
		cvShowImage("lines", lines);

		// calculate total length over all contours
		float contourLengthSum = 0.0f;
		for(CvSeq *c = contours; c; c = c->h_next)
		{
			for(int i = 0; i < c->total - 1; i++)
			{
				CvPoint *p0 = CV_GET_SEQ_ELEM(CvPoint, c, i);
				CvPoint *p1 = CV_GET_SEQ_ELEM(CvPoint, c, i + 1);
				int dx = p1->x - p0->x;
				int dy = p1->y - p0->y;
				contourLengthSum += sqrtf(dx * dx + dy * dy);
			}
		}
		float factor = (float)outputPixelCount / contourLengthSum;

		// write output image
		int cx = 0, cy = 0;
		for(CvSeq *c = contours; c; c = c->h_next)
		{
			for(int i = 0; i < c->total - 1; i++)
			{
				CvPoint *p0 = CV_GET_SEQ_ELEM(CvPoint, c, i);
				CvPoint *p1 = CV_GET_SEQ_ELEM(CvPoint, c, i + 1);
				int dx = p1->x - p0->x;
				int dy = p1->y - p0->y;
				float l = sqrtf(dx * dx + dy * dy);
				float lf = l * factor;
				int n = (int)lf;
				for (int j = 0; j < n; j++)
				{ // TODO: pointerz!
					float t = (float)j / (float)(n - 1);
					outData[cy * outStep + cx * outChannels + 1] = (int)((p0->x + t * dx) / frame->width * 255.0f);
					outData[cy * outStep + cx * outChannels + 2] = (int)(255.0f - (p0->y + t * dy) / frame->height * 255.0f);
					cx++;
					if (cx == width)
					{
						cx = 0;
						cy++;
						if (cy == height)
							goto full;
					}
				}
			}
		}

		// fill last few pixels with last pixel value, if there are any left
		int lastX = outData[cy * outStep + cx * outChannels + 1];
		int lastY = outData[cy * outStep + cx * outChannels + 2];
		for (; cy < height; cy++)
		{
			for (;cx < width; cx++)
			{
				outData[cy * outStep + cx * outChannels + 1] = lastX;
				outData[cy * outStep + cx * outChannels + 2] = lastY;
			}
		}

		full:
		cvReleaseMemStorage(&storage);
		cvShowImage("out", out);

		//usleep(10000); // TODO: wait for next frame timestamp or output a video?
	}
	cvReleaseImage(&out);
	cvReleaseImage(&lines);
	cvReleaseImage(&edges);
	cvReleaseCapture(&cap);
	return 0;
}
