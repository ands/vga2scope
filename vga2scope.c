#include <stdio.h>
#include <unistd.h>
#include <cv.h>
#include <highgui.h>

int main(int argc, char **argv)
{
	// get command line parameters
	if(argc < 6)
	{
		printf("Usage: %s <video> <vga window x> <vga window y> <vga width> <vga height>\n", argv[0]);
		printf("Usage: %s <camera id> <vga window x> <vga window y> <vga width> <vga height>\n", argv[0]);
		exit(0);
	}
	const char *filename = argv[1];
	int vgaX = atoi(argv[2]);
	int vgaY = atoi(argv[3]);
	int vgaWidth = atoi(argv[4]);
	int vgaHeight = atoi(argv[5]);
	int outputPixelCount = vgaWidth * vgaHeight;

	// opencv initializations
	CvCapture* cap = cvCaptureFromFile(filename);
	int isVideoFile = cap != 0;
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
	IplImage* out = cvCreateImage(cvSize(vgaWidth, vgaHeight), IPL_DEPTH_8U, 3);
	int outStep = out->widthStep;
	int outChannels = out->nChannels;
	unsigned char *outData = (unsigned char*)out->imageData;

	// position windows // TODO: make debug output windows optional?
	cvNamedWindow("frame", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("frame", 0, 32);
	cvNamedWindow("edges", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("edges", frame->width, 32);
	cvNamedWindow("lines", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("lines", 2 * frame->width, 32);
	cvNamedWindow("out", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("out", vgaX, vgaY);

	while (42)
	{
		frame = cvQueryFrame(cap);
		if (!frame || (cvWaitKey(1) & 0xff) == 'q')
			break;
		cvShowImage("frame", frame);

		// edge detection
		cvCanny(frame, edges, 128.0, 130.0, 3); // TODO: tweakable parameters?
		cvShowImage("edges", edges);

		// get contours
		CvMemStorage *storage = cvCreateMemStorage(0);
		CvSeq *contours;
		int contourCount = cvFindContours(
			edges, storage, &contours, sizeof(CvContour),
			CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
		cvZero(lines);
		cvDrawContours(lines, contours, cvScalar(32, 255, 32, 255), cvScalarAll(0), 100, 1, 8, cvPoint(0, 0));
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
		float xScale = 255.0f / frame->width;
		float yScale = 255.0f / frame->height;
		unsigned char *dp = &outData[1];
		for(CvSeq *c = contours; c; c = c->h_next)
		{
			CvPoint *p0 = CV_GET_SEQ_ELEM(CvPoint, c, 0);
			for(int i = 1; i < c->total; i++)
			{
				CvPoint *p1 = CV_GET_SEQ_ELEM(CvPoint, c, i);
				float x0x1 = p1->x - p0->x;
				float y0y1 = p1->y - p0->y;
				int n = (int)(sqrtf(x0x1 * x0x1 + y0y1 * y0y1) * factor);
				float x =          (float)p0->x * xScale;
				float y = 255.0f - (float)p0->y * yScale;
				float dt = 1.0f / (float)(n - 1);
				float dx = dt *  x0x1 * xScale;
				float dy = dt * -y0y1 * yScale;
				for (int j = 0; j < n; j++)
				{
					dp[0] = (unsigned char)x;
					dp[1] = (unsigned char)y;
					x += dx;
					y += dy;
					dp += outChannels;
					if (++cx == vgaWidth)
					{
						cx = 0;
						dp = &outData[++cy * outStep + 1];
						if (cy == vgaHeight)
							goto full;
					}
				}
				p0 = p1;
			}
		}

		// fill last few pixels with last pixel value, if there are any left
		for (; cy < vgaHeight; cy++)
		{
			for (;cx < vgaWidth; cx++)
			{
				outData[cy * outStep + cx * outChannels + 1] = 0;
				outData[cy * outStep + cx * outChannels + 2] = 0;
			}
			cx = 0;
		}

		full:
		cvReleaseMemStorage(&storage);
		cvShowImage("out", out);

		//if (isVideoFile)
		//	usleep(8000); // TODO: proper synchronization
	}
	cvReleaseImage(&out);
	cvReleaseImage(&lines);
	cvReleaseImage(&edges);
	cvReleaseCapture(&cap);
	return 0;
}
