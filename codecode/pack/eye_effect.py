import cv2
import math
from sys import argv

source = argv[1]
dest = argv[2]
datafile = argv[3]
mode = int(argv[4])

f = open(datafile, 'r')
lines = f.readlines()

def put(img, pos, pos1, scale, scale1):
	opacity = 0.6
	color = [255, 0, 0]
	target = cv2.imread('/home/ubuntu/pack/eye_img/eye.jpg')
	target = cv2.resize(target, (int(target.shape[0] * scale), int(target.shape[0] * scale)), interpolation = cv2.INTER_AREA)
	width = target.shape[0]
	for i in range(width):
		for j in range(width):
			if 255 - target[i][j][0] > 100 and 255 - target[i][j][1] > 100 and 255 - target[i][j][2] > 100:
				for k in range(3):
					x = j - width / 2 + pos[1]
					y = i - width / 2 + pos[0]
					if x >= img.shape[0] or x < 0 or y >= img.shape[1] or y < 0:
						continue
					img[x, y][k] = int((img[x, y][k] - color[k]) * opacity + color[k])
				
	target = cv2.imread('/home/ubuntu/pack/eye_img/eye_r.jpg')
	target = cv2.resize(target, (int(target.shape[0] * scale1), int(target.shape[0] * scale1)), interpolation = cv2.INTER_AREA)
	width = target.shape[0]
	for i in range(width):
		for j in range(width):
			if 255 - target[i][j][0] > 100 and 255 - target[i][j][1] > 100 and 255 - target[i][j][2] > 100:
				for k in range(3):
					x = j - width / 2 + pos1[1]
					y = i - width / 2 + pos1[0]	
					if x >= img.shape[0] or x < 0 or y >= img.shape[1] or y < 0:
						continue				
					img[x, y][k] = int((img[x, y][k] - color[k]) * opacity + color[k])
					
	target = cv2.imread('/home/ubuntu/pack/eye_img/eye.jpg')
	target = cv2.resize(target, (int(target.shape[0] * (2 * scale1 - scale)), int(target.shape[0] * (2 * scale1 - scale))), interpolation = cv2.INTER_AREA)
	width = target.shape[0]
	for i in range(width):
		for j in range(width):
			if 255 - target[i][j][0] > 100 and 255 - target[i][j][1] > 100 and 255 - target[i][j][2] > 100:
				for k in range(3):
					x = j - width / 2 + (2 * pos1[1] - pos[1])
					y = i - width / 2 + (2 * pos1[0] - pos[0])
					if x >= img.shape[0] or x < 0 or y >= img.shape[1] or y < 0:
						continue
					img[x, y][k] = int((img[x, y][k] - color[k]) * opacity + color[k])


def put2(img, pos, pos1, scale, scale1):
	thres = 1
	opacity = 0.6
	color = [255, 0, 0]
	target = cv2.imread('/home/ubuntu/pack/eye_img/eye2.jpg')
	target = cv2.resize(target, (int(target.shape[0] * scale), int(target.shape[0] * scale)), interpolation = cv2.INTER_AREA)
	width = target.shape[0]
	
	for i in range(width):
		for j in range(width):
			#print 95 - target[i][j][0], 93 - target[i][j][1], 96 - target[i][j][2]
			if abs(95 - target[i][j][0]) > thres and abs(93 - target[i][j][1]) > thres and abs(96 - target[i][j][2]) > thres:
				for k in range(3):
					img[j - width / 2 + pos[1], i - width / 2 + pos[0]][k] = int((img[j - width / 2 + pos[1], i - width / 2 + pos[0]][k] - color[k]) * opacity + color[k])
				
	target = cv2.imread('/home/ubuntu/pack/eye_img/eye2_r.jpg')
	target = cv2.resize(target, (int(target.shape[0] * scale1), int(target.shape[0] * scale1)), interpolation = cv2.INTER_AREA)
	width = target.shape[0]
	for i in range(width):
		for j in range(width):
			if abs(95 - target[i][j][0]) > thres and abs(93 - target[i][j][1]) > thres and abs(96 - target[i][j][2]) > thres:
				for k in range(3):
					img[j - width / 2 + pos1[1], i - width / 2 + pos1[0]][k] = int((img[j - width / 2 + pos1[1], i - width / 2 + pos1[0]][k] - color[k]) * opacity + color[k])
				
	target = cv2.imread('/home/ubuntu/pack/eye_img/eye2.jpg')
	target = cv2.resize(target, (int(target.shape[0] * (2 * scale1 - scale)), int(target.shape[0] * (2 * scale1 - scale))), interpolation = cv2.INTER_AREA)
	width = target.shape[0]
	for i in range(width):
		for j in range(width):
			if abs(95 - target[i][j][0]) > thres and abs(93 - target[i][j][1]) > thres and abs(96 - target[i][j][2]) > thres:
				for k in range(3):
					img[j - width / 2 + (2 * pos1[1] - pos[1]), i - width / 2 + (2 * pos1[0] - pos[0])][k] = int((img[j - width / 2 + (2 * pos1[1] - pos[1]), i - width / 2 + (2 * pos1[0] - pos[0])][k] - color[k]) * opacity + color[k])

def put3(img, pos, scale, loop):
	thres = 1
	target = cv2.imread('/home/ubuntu/pack/eye_img/pupil.jpg')
	target = cv2.resize(target, (int(target.shape[1] * scale), int(target.shape[0] * scale)), interpolation = cv2.INTER_AREA)
	width = target.shape[0]
	for i in range(width):
		for j in range(width):
			if target[i][j][0] > thres and target[i][j][1] > thres and target[i][j][2] > thres:
				for k in range(3):
					img[j - width / 2 + pos[1], i - width / 2 + pos[0]][k] = target[i][j][(k + loop) % 3]
					
def darken(img, scale):
	for i in range(img.shape[0]):
		for j in range(img.shape[1]):
			img[i][j][0] = int(img[i][j][0] * scale)
			img[i][j][1] = int(img[i][j][1] * scale)
			img[i][j][2] = int(img[i][j][2] * scale)

count = 0

videoCapture = cv2.VideoCapture(source)
fps = videoCapture.get(cv2.cv.CV_CAP_PROP_FPS)
size = (int(videoCapture.get(cv2.cv.CV_CAP_PROP_FRAME_WIDTH)), 
        int(videoCapture.get(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT)))
videoWriter = cv2.VideoWriter(dest, cv2.cv.CV_FOURCC('I', '4', '2', '0'), fps, size)
success, img = videoCapture.read()
print len(lines)
count = 0
eyeclose = True
loop = 0
while success:
	if lines[count + 1] == '\n':
		count += 2
		videoWriter.write(img)
		success, img = videoCapture.read()
		continue
	line = []
	for i in range(11):
		line.append(lines[count + 1 + i].split(' ')[0])
		line[-1] = str(int(float(line[-1])))
		line.append(lines[count + 1 + i].split(' ')[1][:-1])
		line[-1] = str(int(float(line[-1])))
	count += 13
	le = (int(line[0]), int(line[1]))
	re = (int(line[2]), int(line[3]))
	no = (int(line[12]), int(line[13]))
	lorbit = [(int(line[14]), int(line[15])), (int(line[18]), int(line[19]))]
	rorbit = [(int(line[16]), int(line[17])), (int(line[20]), int(line[21]))]
	a = [(float(line[7]) - float(line[5])) / (float(line[4]) - float(line[6])), (float(line[11]) - float(line[9])) / (float(line[8]) - float(line[10]))]
	c = [-a[0] * float(line[4]) - float(line[5]), -a[1] * float(line[8]) - float(line[9])]
	d = [float(abs(a[0] * le[0] + le[1] + c[0])) / float(math.sqrt(a[0] ** 2 + 1.)), float(abs(a[1] * le[0] + le[1] + c[1])) / float(math.sqrt(a[1] ** 2 + 1.))]
	
	if (- a[0] * le[0] - c[0] > le[1]):
		up = - d[0] * 17 / 3.34
	else:
		up = d[0] * 17 / 3.34
	
	nd = [float(abs(no[0] - le[0])), float(abs(no[0] - re[0]))]
	#print nd[0] / nd[1], nd[1] / nd[0]
	if nd[0] > nd[1]:
		shift = (nd[0] / nd[1] - 1) * 10 / 0.27216
	else:
		shift = - (nd[1] / nd[0] - 1) * 10 / 0.27216
	
	pos2 = (le[0] + shift, le[1] + up)
	#darken(img, 0.3)
	if mode == 1:
		put(img, le, pos2, 0.2, 0.3)
	elif mode == 2:
		put2(img, le, pos2, 0.2, 0.3)
	elif mode == 3:
		thres_orbit = 100
		print (lorbit[0][0] - lorbit[1][0]) ** 2 + (lorbit[0][1] - lorbit[1][1]) ** 2
		if (lorbit[0][0] - lorbit[1][0]) ** 2 + (lorbit[0][1] - lorbit[1][1]) ** 2 > thres_orbit:
			if eyeclose == False:
				loop += 1
			eyeclose = True
			put3(img, (le[0], le[1] - 5), 0.015, loop)
			put3(img, (re[0], re[1] - 5), 0.015, loop)
		else:
			eyeclose = False
	
	#cv2.imshow("test", img)
	#cv2.waitKey(1)
	videoWriter.write(img)
	success, img = videoCapture.read()
