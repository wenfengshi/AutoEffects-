import cv2
import math
from sys import argv

source = argv[1]
dest = argv[2]
datafile = argv[3]

f = open(datafile, 'r')
lines = f.readlines()
videoCapture = cv2.VideoCapture(source)
fps = videoCapture.get(cv2.cv.CV_CAP_PROP_FPS)
size = (int(videoCapture.get(cv2.cv.CV_CAP_PROP_FRAME_WIDTH)), 
        int(videoCapture.get(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT)))
videoWriter = cv2.VideoWriter(dest, cv2.cv.CV_FOURCC('I', '4', '2', '0'), fps, size)

def check(arm1, arm2):
	thres = 0.05
	check1 = False
	check2 = False
	v1 = [arm1[0][0] - arm1[1][0], arm1[0][1] - arm1[1][1]]
	v2 = [arm1[2][0] - arm1[1][0], arm1[2][1] - arm1[1][1]]
	try:
		if abs(math.atan(float(v1[1]) / float(v1[0])) - math.atan(float(v2[1]) / float(v2[0]))) < thres:
			check1 = True
	except:
		if abs(math.atan(float(v1[0]) / float(v1[1])) - math.atan(float(v2[0]) / float(v2[1]))) < thres:
			check1 = True
	v1 = [arm2[0][0] - arm2[1][0], arm2[0][1] - arm2[1][1]]
	v2 = [arm2[2][0] - arm2[1][0], arm2[2][1] - arm2[1][1]]
	try:
		if abs(math.atan(float(v1[1]) / float(v1[0])) - math.atan(float(v2[1]) / float(v2[0]))) < thres:
			check2 = True
	except:
		if abs(math.atan(float(v1[0]) / float(v1[1])) - math.atan(float(v2[0]) / float(v2[1]))) < thres:
			check2 = True
	return check1 and check2

def put(img, pos, scale, loop):
	opacity = 0.1
	target = cv2.imread("/home/ubuntu/pack/fire_img/" + str(loop) + '.jpg')
	target = cv2.resize(target, (int(target.shape[1] * scale), int(target.shape[0] * scale)), interpolation = cv2.INTER_AREA)
	thres = 20
	pos[1] += 35
	#print pos

	#print img.shape
	
	for i in range(target.shape[1]):
		for j in range(target.shape[0]):
			x = pos[0] - i + (target.shape[1] / 2)
			y = pos[1] - (target.shape[0] - j)
			if x >= img.shape[1] or x < 0 or y >= img.shape[0] or y < 0:
				continue
			if not(target[j][i][2] > thres and target[j][i][1] > thres):
				continue
			# print target[j][i]
			'''
			for k in range(3):
				img[y, x][k] = int((img[y, x][k] - target[j][i][k]) * opacity + target[j][i][k])
			'''
			img[y, x][0] = 0
			img[y, x][1] = 0
			img[y, x][2] = 255

success, img = videoCapture.read()
count = 0
trigger = 0
frame = 0
loop = 0
while success:
	frame += 1
	if lines[count + 1] == '\n':
		count += 2
		videoWriter.write(img)
		success, img = videoCapture.read()
		continue
	
	arm1 = []
	arm2 = []
	center = []
	for i in range(3):
		x = int(float(lines[count + 1 + i].split(' ')[0]))
		y = int(float(lines[count + 1 + i].split(' ')[1][:-1]))
		arm1.append([x, y])
	for i in range(3, 6):
		x = int(float(lines[count + 1 + i].split(' ')[0]))
		y = int(float(lines[count + 1 + i].split(' ')[1][:-1]))
		arm2.append([x, y])
	center = [int(float(lines[count + 7].split(' ')[0])), int(float(lines[count + 7].split(' ')[1][:-1]))]
	
	if trigger == 0:
		if check(arm1, arm2):
			trigger = 1
		videoWriter.write(img)
	else:
		if loop > 9:
			loop = 6
		put(img, arm1[2], 0.07, loop)
		put(img, arm2[2], 0.07, loop)
		# cv2.circle(img, (arm1[2][0], arm1[2][1]), 2, (255, 0, 0), 2)
		loop += 1
		videoWriter.write(img)
	count += 9
	success, img = videoCapture.read()
