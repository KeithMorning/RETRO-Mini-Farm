import AVFoundation
import CoreVideo
import Foundation

let args = CommandLine.arguments
guard args.count == 3 else {
    print("usage: encode_video input.rgb output.mp4")
    exit(1)
}
let inputPath = args[1]
let outputPath = args[2]
let width = 320
let height = 240
let fps: Int32 = 30
let frameBytes = width * height * 3

let data = try Data(contentsOf: URL(fileURLWithPath: inputPath), options: [.mappedIfSafe])
guard data.count % frameBytes == 0 else {
    print("bad rgb size")
    exit(1)
}
let frames = data.count / frameBytes

let outputURL = URL(fileURLWithPath: outputPath)
try? FileManager.default.removeItem(at: outputURL)
let writer = try AVAssetWriter(outputURL: outputURL, fileType: .mp4)
let settings: [String: Any] = [
    AVVideoCodecKey: AVVideoCodecType.h264,
    AVVideoWidthKey: width,
    AVVideoHeightKey: height,
    AVVideoCompressionPropertiesKey: [
        AVVideoAverageBitRateKey: 1_800_000,
        AVVideoProfileLevelKey: AVVideoProfileLevelH264MainAutoLevel
    ]
]
let videoInput = AVAssetWriterInput(mediaType: .video, outputSettings: settings)
videoInput.expectsMediaDataInRealTime = false
let attrs: [String: Any] = [
    kCVPixelBufferPixelFormatTypeKey as String: kCVPixelFormatType_32BGRA,
    kCVPixelBufferWidthKey as String: width,
    kCVPixelBufferHeightKey as String: height
]
let adaptor = AVAssetWriterInputPixelBufferAdaptor(assetWriterInput: videoInput, sourcePixelBufferAttributes: attrs)
guard writer.canAdd(videoInput) else { print("cannot add video"); exit(1) }
writer.add(videoInput)
guard writer.startWriting() else { print("start failed: \(String(describing: writer.error))"); exit(1) }
writer.startSession(atSourceTime: .zero)

var pool: CVPixelBufferPool?
CVPixelBufferPoolCreate(kCFAllocatorDefault, nil, attrs as CFDictionary, &pool)

data.withUnsafeBytes { (raw: UnsafeRawBufferPointer) in
    for i in 0..<frames {
        while !videoInput.isReadyForMoreMediaData {
            usleep(1000)
        }
        var buffer: CVPixelBuffer?
        if let p = pool {
            CVPixelBufferPoolCreatePixelBuffer(kCFAllocatorDefault, p, &buffer)
        } else {
            CVPixelBufferCreate(kCFAllocatorDefault, width, height, kCVPixelFormatType_32BGRA, nil, &buffer)
        }
        guard let pb = buffer else { continue }
        CVPixelBufferLockBaseAddress(pb, [])
        let base = CVPixelBufferGetBaseAddress(pb)!.assumingMemoryBound(to: UInt8.self)
        let stride = CVPixelBufferGetBytesPerRow(pb)
        let src = raw.baseAddress!.assumingMemoryBound(to: UInt8.self)
            .advanced(by: i * frameBytes)
        for y in 0..<height {
            for x in 0..<width {
                let s = (y * width + x) * 3
                let d = y * stride + x * 4
                base[d + 0] = src[s + 2] // B
                base[d + 1] = src[s + 1] // G
                base[d + 2] = src[s + 0] // R
                base[d + 3] = 255
            }
        }
        CVPixelBufferUnlockBaseAddress(pb, [])
        adaptor.append(pb, withPresentationTime: CMTime(value: Int64(i), timescale: fps))
    }
}

videoInput.markAsFinished()
let sem = DispatchSemaphore(value: 0)
writer.finishWriting { sem.signal() }
sem.wait()
if writer.status != .completed {
    print("write failed: \(String(describing: writer.error))")
    exit(1)
}
print("wrote \(outputPath) (\(frames) frames)")
